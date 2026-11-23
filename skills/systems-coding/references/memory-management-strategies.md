# Memory management: choosing allocation strategies

- [Core message](#core-message)
- [Worked layout: a bounded search tab](#worked-layout-a-bounded-search-tab)
- [Questions that choose the strategy](#questions-that-choose-the-strategy)
- [Selection map](#selection-map)
- [Implementation patterns](#implementation-patterns)
- [Practical constraints not to hide](#practical-constraints-not-to-hide)
- [Working default](#working-default)
- [Source material](#source-material)

## Core message

Use knowledge of data to simplify its memory management. General-purpose allocation must handle unrelated sizes, lifetimes, and access patterns. Application-specific allocation can often avoid that complexity.

The useful distinction is not manual versus automatic memory management. It is general-purpose management supplied by an environment versus management built around an application's data. Both can automate allocation and cleanup.

Four patterns carry the main practical insight:

1. **Group by lifetime:** Give each tab or document an arena for its related data. Close owner, release its arena.
2. **Choose limits up front:** Bound record count and backing storage before inserting data. Avoid relocation on growth; define what happens when capacity is reached.
3. **Pack by actual size:** Keep filename descriptors in compact structs and filename bytes in a separate bump allocator. Do not embed a maximum-length character array in every record.
4. **Reuse dead storage for bookkeeping:** When equal-sized slots become free, store free-list links inside those slots rather than allocating a second structure to track them.

These patterns simplify ownership and reduce wasted space together. They are more useful as a composed design than as an isolated list of allocator types. Do not force them onto workloads whose lifetimes, sizes, or reference requirements do not fit.

## Worked layout: a bounded search tab

```text
Application
  Shared frame arena: temporary display strings; reset after frame consumers finish

  Tab A arena: release when Tab A closes
    Query and tab state
    File descriptors: bounded array of small records
      Each filename: pointer into path storage plus length
    Path storage: bounded bump allocator
      Actual filename bytes packed consecutively

  Tab B arena: same layout, independent lifetime
```

Choose maximum file count and path-byte budget when opening a tab. Carve descriptor storage and path storage from that tab's arena. Adding a file consumes one descriptor slot and only the bytes needed for its path. The structs and strings live in separate regions, but share the tab's cleanup point.

The important distinction is **maximum capacity of the whole buffer**, not **maximum-sized storage inside every element**. A large bounded path buffer can hold short paths tightly packed. Embedding a maximum-length path in each record leaves repeated unused gaps between useful data. An arena alone does not fix that layout.

If a collection needs frequent independent deletion, use reusable fixed-size slots with in-place free-list links for that collection. This does not reclaim arbitrary filename byte ranges in the path bump allocator; those need a separate policy. Keep lifetime grouping, layout, and reclamation distinct.

## Questions that choose the strategy

Answer these before choosing an allocator or container:

1. **Lifetime:** Does data live for the whole program, one operation, one frame, one document, or an independent duration?
2. **Size:** Is size known at compile time, known when allocating, or changed afterward?
3. **Capacity:** What maximum must the product support? What happens when reached?
4. **References:** Do other objects retain pointers to elements? May elements move?
5. **Order:** Must iteration preserve insertion order or another meaningful order?
6. **Mutation:** How many additions, deletions, and resizes occur, and how often?
7. **Ownership:** Can one owner release a group, or must objects be released separately?

Choose containers and allocation together. Deletion policy can matter more than the underlying allocator.

## Selection map

| Data or operation | Start with | Main trade-off |
| --- | --- | --- |
| Fixed-size state lasting the whole program | Static storage | Simple lifetime; fixed capacity and global-state risks |
| Size known at startup, unchanged afterward | One startup allocation | Simple ownership; storage remains until explicitly released or process exit |
| Bounded collection whose elements need stable addresses | Fixed-capacity array | No relocation on append; must handle capacity exhaustion |
| Variable-length strings collected for one operation | Descriptor array plus linear allocator | Dense storage; cannot grow individual allocations in place without spare capacity |
| Scratch values discarded together each frame or operation | Resettable arena | Cheap bulk reset; no retained references after reset |
| A document, tab, request, or job owns many allocations | Owner-scoped arena | One cleanup point; individual dead allocations may remain until owner closes |
| Temporary allocations released in reverse order | Stack allocator or arena marks | Cheap rewind; lifetime nesting must be correct |
| Unordered array with frequent deletion | Swap-back removal | Compact storage; removal moves another element |
| Ordered results with rare deletion | Tombstones or deleted flags | Stable storage; dead entries consume capacity and iteration work |
| Rare edits to tightly packed variable-length values | Allocate replacement and redirect reference | Simple update; old storage remains until reset or reclamation |
| Same-size objects with independent deletion and reuse | Pool with free list | Stable live slots; stale references can alias reused objects |
| Changing sizes or independent lifetimes without a useful grouping | General-purpose allocation or growable containers | Flexible; resizing and per-object ownership need explicit contracts |

## Implementation patterns

### 1. Static storage or one startup allocation

**Use when:** Data lasts for the whole program. Use static storage when its size is known at compile time; allocate at startup when size becomes known then.

**Shape:** Give state an owner, allocate once, and keep its address unchanged. A configuration file loaded once is a typical example.

**Benefits:** No resize policy, relocation, or repeated allocation. Process-lifetime data need not be individually released immediately before process exit.

**Costs and limits:** Static capacity cannot adapt to larger input. Global mutable state can entangle otherwise independent parts of a program. Process-exit cleanup does not cover files, external transactions, or other non-memory responsibilities.

**Change strategy when:** Data must unload or reload while the program continues, multiple instances need separate state, or expected capacity becomes unreasonable.

### 2. Fixed-capacity array with checked access

**Use when:** Maximum count is meaningful and elements have equal size. Especially useful when appends must not invalidate retained pointers.

**Shape:** Store `data`, `length`, and `capacity` together. Append only if `length < capacity`. Read only if `index < length`; reject negative indexes before conversion to unsigned types.

**Benefits:** Contiguous iteration, simple bounds checks, and no relocation on append. Storage can be static, allocated once, or carved from an arena.

**Costs and limits:** Capacity becomes a product contract. Ordered deletion still requires movement unless using a different deletion policy. Stable backing storage does not make an element's identity stable if later operations move or reuse its slot.

**Change strategy when:** Capacity is genuinely unpredictable or so large that the chosen backing allocation is not viable. Do not silently truncate results or overwrite existing elements.

### 3. Descriptor array plus linear allocation for variable-length data

**Use when:** Collecting strings or blobs whose individual sizes become known at insertion and whose contents mostly remain unchanged afterward.

**Shape:** Keep small descriptors such as pointer and length in one array. Keep actual bytes in a separate contiguous buffer with capacity and used length. Append each value's bytes at the current used offset; advance by its actual size.

**Benefits:** Packs variable-length values without reserving maximum-sized inline storage for every element. Descriptors remain compact. Individual byte ranges need not move.

**Costs and limits:** Descriptor capacity and byte capacity are separate limits. Removing a descriptor does not reclaim its byte range. Growing a value may require replacement storage. A null-terminated API also needs room for the terminator.

**Avoid:** Moving or resizing the byte buffer while descriptors retain raw pointers into it. Use stable backing storage, or a different reference representation with an explicit relocation policy.

**Change strategy when:** Frequent edits and deletions leave too much dead storage before the collection can reset.

### 4. Resettable scratch or frame arena

**Use when:** Many temporary values share a clear end point: one render frame, parse, query, or operation.

**Shape:** Allocate a buffer once. Each allocation advances its used offset. Reset used offset to zero only after all consumers have finished.

**Benefits:** Simple allocation and bulk cleanup. No per-object free bookkeeping. Useful for generating derived display strings on demand rather than maintaining a second persistent representation.

**Costs and limits:** Reset invalidates every allocation. Peak simultaneous scratch demand determines capacity. Nested or asynchronous consumers may outlive the assumed cleanup point.

**Example:** Keep a file's timestamp as authoritative data. Format strings only for visible rows into frame scratch. Regenerate next frame rather than keeping every formatted string synchronized with the timestamp.

**Change strategy when:** Some results must survive reset. Copy those results to longer-lived storage, or give their owner a separate arena.

### 5. Owner-scoped arena

**Use when:** One tab, document, request, or job owns many related allocations that can all be discarded together.

**Shape:** Allocate an arena for the owner. Carve internal arrays, buffers, and substructures from it. Release the outer arena when owner closes.

**Benefits:** Cleanup follows ownership rather than requiring a matching free for every internal allocation. Internal complexity can grow without growing the number of memory cleanup points.

**Costs and limits:** An arena is a lifetime policy, not a promise of unlimited space. Dead internal allocations can remain for the owner's entire lifetime. Fixed-size backing requires a bound; growing backing requires a deliberate reference-stability policy.

**Example:** Search tab owns query, matched-file descriptors, and path bytes. Frame scratch stays outside tab because its lifetime is one frame, not one tab.

**Change strategy when:** Owner is long-lived and repeatedly replaces large portions of its contents. Consider sub-arenas with shorter lifetimes or a reclaiming container for those portions.

### 6. Stack allocation or arena marks

**Use when:** Temporary allocations have nested lifetimes and can be released in reverse order.

**Shape:** Save current used offset before a nested operation. Allocate its temporaries. Restore saved offset after its consumers finish, including error paths.

**Benefits:** Cheap bulk rewind without freeing each temporary. Supports nested scratch use without resetting unrelated earlier allocations.

**Costs and limits:** Returned pointers cannot outlive the restored mark. Restoring a mark too early invalidates live data; forgetting to restore it accumulates temporary storage.

Use ordinary function-local variables for ordinary local state. The video's custom call-stack exercise explains a pattern; it is not a recommendation to replace the language's call stack.

### 7. Choose a deletion policy

#### Swap-back removal

Replace removed element with final element, then decrement length.

**Use when:** Elements have equal size, order is irrelevant, and references to the moved element can be absent or updated.

**Trade-off:** Constant number of element copies, but changed ordering and element location. Good for some particle or point collections; unsuitable for ordered strings or untracked retained pointers.

#### Tombstones or deleted flags

Mark entry deleted. Skip it during iteration, or display its deleted state.

**Use when:** Deletions are rare relative to collection size, stable locations matter, and collection resets at a reasonable point.

**Trade-off:** Avoids compaction and pointer repairs, but retains dead storage and makes iteration consider deleted entries. Can support a useful undo or restore experience.

**Reconsider when:** Collection runs indefinitely or deletion churn approaches capacity. Compaction, rebuilding, or reusable slots need an explicit policy then.

#### Allocate replacement and redirect

Append replacement bytes elsewhere and update owning descriptor. Leave old bytes unused until reset.

**Use when:** Occasional rename or edit can outgrow a tightly packed allocation.

**Trade-off:** Avoids moving neighboring values. Uses additional capacity and requires clear ownership of references to the old value. Updating one descriptor does not update arbitrary external pointers.

These policies change the problem rather than trying to solve every possible resize and deletion in place. Check whether that simpler behavior matches actual product needs.

### 8. Pool with free list

**Use when:** Many equal-sized objects need independent allocation and deletion while live objects keep stable locations.

**Shape:** Back pool with stable, equal-sized slots. Keep one free-list head. Each free slot stores the next free slot's pointer or index in storage that no longer holds live data. A slot must have enough size and suitable alignment for that link.

- **Release:** End object's live use, write current free-list head into its slot, then make that slot the new head.
- **Allocate:** Take head slot, advance head to its stored next link, then initialize slot as live data. If no free slot exists, use a never-used slot within capacity or report exhaustion.

The same backing storage alternates between live objects and free-list nodes. No separate array of free indexes is required. This reuses holes; it does not move live objects or compact them into one contiguous run.

**Benefits:** Reuses deleted storage without moving other live objects. Allocation and release need only bounded bookkeeping. One backing allocation can simplify whole-pool cleanup.

**Costs and limits:** Capacity is still bounded. Free slots cannot hold live data. Double release can corrupt free-list bookkeeping. A pointer to a deleted slot may later refer to a different object placed there.

Logical ordering is a separate decision. A linked list can provide ordering without moving slots, but pointer-based traversal may have worse locality than a compact array. A pool does not require its live objects to form a linked list.

**Change strategy when:** Objects vary substantially in size, capacity cannot be bounded reasonably, or consumers require compact contiguous live data more than stable addresses.

### 9. General-purpose allocation and growable containers

**Use when:** Data has independent lifetimes, sizes vary, or no simpler lifetime grouping fits. Existing library containers may be the smallest correct implementation.

**Benefits:** Flexible sizing and familiar interfaces. Avoids inventing specialized allocation machinery for a workload that does not benefit from it.

**Costs and limits:** Individual ownership and cleanup remain necessary. A growable array may relocate its elements on growth; retained pointers must obey its invalidation rules. Allocation behavior may be unsuitable for a latency-sensitive path without measurement.

Prefer specialization when a concrete bound, lifetime, or latency requirement makes the simpler specialized policy worthwhile—not merely because general-purpose allocation exists.

## Practical constraints not to hide

- **Capacity is not free memory.** A large virtual allocation does not guarantee that physical backing or later writes will succeed. Platform behavior differs, especially between Windows and the POSIX examples. Budget actual demand and handle allocation failure.
- **Dense layout matters.** Reserving a large outer buffer does not rescue a layout that wastes most of every occupied region. Pack variable-length data by actual used size when appropriate.
- **Check arithmetic and alignment.** Validate multiplication, offset growth, alignment padding, and remaining capacity before writing. These apply even to a minimal linear allocator.
- **Stable address is not permanent validity.** Reset, destruction, compaction, replacement, and slot reuse can invalidate a reference for different reasons. State the contract for each.
- **Memory cleanup is not resource cleanup.** Arena release alone does not close files, release GPU resources, run required destructors, or cancel outstanding work.
- **Concurrency changes ownership.** Do not reset or recycle storage while another thread or asynchronous operation may still use it.

The videos supply the main strategies. The explicit ownership, failure, resource-cleanup, and concurrency cautions here are implementation guidance, not claims that the videos fully cover those subjects.

## Working default

1. Start with the shortest useful lifetime and one clear owner.
2. Use existing containers unless a simpler specialized representation fits actual data better.
3. Prefer one allocation or arena when data can be released together.
4. Use bounded arrays when a supported maximum and stable storage are useful.
5. Choose deletion policy from ordering, reference, and churn requirements.
6. Add reuse or individual reclamation only where retained dead storage becomes a real problem.
7. Test full capacity, failed allocation, repeated edits/deletions, reset, and retained-reference behavior. Measure representative peak memory and latency before claiming improvement.

## Source material

Practical synthesis of Nic Barker's *Memory Management, Explained Simply*, Parts 1–3. Not a transcript, API specification, or universal allocator recommendation. OS explanations and teaching exercises are intentionally omitted except where they constrain a choice.

- [Part 1](https://www.youtube.com/watch?v=3rez2rDYHHA): general-purpose versus application-specific management; static and startup allocations.
- [Part 2](https://www.youtube.com/watch?v=7mYWbTgEGb8): bounded arrays, packed strings, linear allocation, tombstones, and replacement updates.
- [Part 3](https://www.youtube.com/watch?v=VW_NUH-6kv0): stack lifetimes, frame and owner arenas, swap-back removal, linked storage, and free lists.
