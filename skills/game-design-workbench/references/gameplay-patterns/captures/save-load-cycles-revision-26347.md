# Save-Load Cycles

_The actions of saving and loading game states to help get the game state one wishes._

Many computer games allow players to save their game state. This can let them divide game instances into multiple game sessions and be able to turn of the game programs in-between. It can also let players store game state when they are in good positions or before they try potentially dangerous actions so they can go back to that game state if things went bad. Doing so repeatedly while playing causes players to engage in Save-Load Cycles.

### Examples

Examples of games that allow Save-Load Cycles include the [Assassin's Creed series](https://gdp.cse.chalmers.se/index.php?title=Assassin%27s_Creed_series "Assassin's Creed series"), the [Baldur's Gate series](https://gdp.cse.chalmers.se/index.php?title=Baldur%27s_Gate_series "Baldur's Gate series"), the [Deus Ex series](https://gdp.cse.chalmers.se/index.php?title=Deus_Ex_series "Deus Ex series"), the [Doom series](https://gdp.cse.chalmers.se/index.php?title=Doom_series "Doom series"), the [Dragon Age series](https://gdp.cse.chalmers.se/index.php?title=Dragon_Age_series "Dragon Age series"), the [Elder Scrolls series](https://gdp.cse.chalmers.se/index.php?title=Elder_Scrolls_series "Elder Scrolls series"), the [Europa Universalis series](https://gdp.cse.chalmers.se/index.php?title=Europa_Universalis_series "Europa Universalis series"), [Mirror's Edge](https://gdp.cse.chalmers.se/index.php?title=Mirror%27s_Edge "Mirror's Edge"), [Sid Meier's Alpha Centauri](https://gdp.cse.chalmers.se/index.php?title=Sid_Meier%27s_Alpha_Centauri "Sid Meier's Alpha Centauri"), the [X-COM series](https://gdp.cse.chalmers.se/index.php?title=X-COM_series "X-COM series"), and the [Zork series](https://gdp.cse.chalmers.se/index.php?title=Zork_series "Zork series"). Of these, [Europa Universalis series](https://gdp.cse.chalmers.se/index.php?title=Europa_Universalis_series "Europa Universalis series") and [X-COM series](https://gdp.cse.chalmers.se/index.php?title=X-COM_series "X-COM series") provide "ironman modes" in which the Save-Load Cycles are forbidden to make the games more challenging.

## Using the pattern

While it is used both for [Turn-Based Games](https://gdp.cse.chalmers.se/index.php?title=Turn-Based_Games "Turn-Based Games") and [Real-Time Games](https://gdp.cse.chalmers.se/index.php?title=Real-Time_Games "Real-Time Games"), the presence of Save-Load Cycles affect the latter more through allowing [Game Pauses](https://gdp.cse.chalmers.se/index.php?title=Game_Pauses "Game Pauses") in them which otherwise would not be present ([Turn-Based Games](https://gdp.cse.chalmers.se/index.php?title=Turn-Based_Games "Turn-Based Games") have the opportunity for [Game Pauses](https://gdp.cse.chalmers.se/index.php?title=Game_Pauses "Game Pauses") between each turn naturally). The primary challenge of Save-Load Cycles is the technical need of being able to store whole game states and quickly replace current ones with saved ones. This is typically solved by having games that support [Save Files](https://gdp.cse.chalmers.se/index.php?title=Save_Files "Save Files") or are run by [Dedicated Game Facilitators](https://gdp.cse.chalmers.se/index.php?title=Dedicated_Game_Facilitators "Dedicated Game Facilitators"). [Save Points](https://gdp.cse.chalmers.se/index.php?title=Save_Points "Save Points") can be used to make this easier but can also be used to require players to get to certain point in the gameplay before being allowed to save.

Save-Load Cycles can difficult to combine well with [Multiplayer Games](https://gdp.cse.chalmers.se/index.php?title=Multiplayer_Games "Multiplayer Games") since this requires that all players can easily come to agreements on when to abandon current play sessions and replace the current game state with a saved one. This becomes a practical impossibility in [Massively Multiplayer Online Games](https://gdp.cse.chalmers.se/index.php?title=Massively_Multiplayer_Online_Games "Massively Multiplayer Online Games") since these typically wish to provide [Persistent Game Worlds](https://gdp.cse.chalmers.se/index.php?title=Persistent_Game_Worlds "Persistent Game Worlds") for all players.

Some [Difficulty Levels](https://gdp.cse.chalmers.se/index.php?title=Difficulty_Levels "Difficulty Levels"), typically called "ironman mode", forbid the use of Save-Load Cycles. This is typically done too make the games more difficult but there are other reasons to consider not providing Save-Load Cycles. They void [Irreversible Events](https://gdp.cse.chalmers.se/index.php?title=Irreversible_Events "Irreversible Events") and make [Penalties](https://gdp.cse.chalmers.se/index.php?title=Penalties "Penalties") such as [Death Consequences](https://gdp.cse.chalmers.se/index.php?title=Death_Consequences "Death Consequences") and [Permadeath](https://gdp.cse.chalmers.se/index.php?title=Permadeath "Permadeath") avoidable. Further, they let players experience [Leaps of Faith](https://gdp.cse.chalmers.se/index.php?title=Leaps_of_Faith "Leaps of Faith"), [Surprises](https://gdp.cse.chalmers.se/index.php?title=Surprises&action=edit&redlink=1 "Surprises (page does not exist)"), and [Tension](https://gdp.cse.chalmers.se/index.php?title=Tension "Tension") once and then reload a play again without having to be affect by these emotionally. They can also make [Time Limited Game Instances](https://gdp.cse.chalmers.se/index.php?title=Time_Limited_Game_Instances "Time Limited Game Instances") pointless at least from a planning perspective and stop [Scores](https://gdp.cse.chalmers.se/index.php?title=Scores "Scores") from representing how well players played in a continuous series of play sessions.

### Diegetic Aspects

Save-Load Cycles are typically not explained through a game's diegesis. An exception can be found in the [Assassin's Creed series](https://gdp.cse.chalmers.se/index.php?title=Assassin%27s_Creed_series "Assassin's Creed series").

### Interface Aspects

While access to "quick-save" and "quick-load" functionality may be enabled through the primary interface of a game, the main functionality for Save-Load Cycles is typically provided in [Secondary Interface Screens](https://gdp.cse.chalmers.se/index.php?title=Secondary_Interface_Screens "Secondary Interface Screens").

### Narration Aspects

Besides potentially making the presence of [Irreversible Events](https://gdp.cse.chalmers.se/index.php?title=Irreversible_Events "Irreversible Events"), [Leaps of Faith](https://gdp.cse.chalmers.se/index.php?title=Leaps_of_Faith "Leaps of Faith"), and [Surprises](https://gdp.cse.chalmers.se/index.php?title=Surprises&action=edit&redlink=1 "Surprises (page does not exist)") mute regarding gameplay, Save-Load Cycles can do the same for narration aspects of a game.

## Consequences

The actual actions of saving and loading are [Extra-Game Actions](https://gdp.cse.chalmers.se/index.php?title=Extra-Game_Actions "Extra-Game Actions") and cause some [Downtime](https://gdp.cse.chalmers.se/index.php?title=Downtime "Downtime") for players. However, more interesting is the many effects on gameplay Save-Load Cycles have. The first most basic consequences is that it allows for [Interruptibility](https://gdp.cse.chalmers.se/index.php?title=Interruptibility "Interruptibility") since players can have [Game Pauses](https://gdp.cse.chalmers.se/index.php?title=Game_Pauses "Game Pauses") and this gives players a [Freedom of Choice](https://gdp.cse.chalmers.se/index.php?title=Freedom_of_Choice "Freedom of Choice") when to play since the pattern in practice gives them [Negotiable Play Sessions](https://gdp.cse.chalmers.se/index.php?title=Negotiable_Play_Sessions "Negotiable Play Sessions"). Player can also use the interludes between play sessions to plan, so the pattern also provides [Stimulated Planning](https://gdp.cse.chalmers.se/index.php?title=Stimulated_Planning "Stimulated Planning"). However, the use of the pattern in a game breaks [Temporal Consistency](https://gdp.cse.chalmers.se/index.php?title=Temporal_Consistency "Temporal Consistency") of the diegesis as compared to the sequence of gameplay actions players perform.

The ability to load after something unwanted has occurred gives players a form of [Reversibility](https://gdp.cse.chalmers.se/index.php?title=Reversibility&action=edit&redlink=1 "Reversibility (page does not exist)") and [Extra Chances](https://gdp.cse.chalmers.se/index.php?title=Extra_Chances "Extra Chances"), and since players can learn from previous attempts Save-Load Cycles provide [Trans-Game Information](https://gdp.cse.chalmers.se/index.php?title=Trans-Game_Information&action=edit&redlink=1 "Trans-Game Information (page does not exist)") between play sessions and can let players create [Fudged Results](https://gdp.cse.chalmers.se/index.php?title=Fudged_Results "Fudged Results") for themselves. Games providing [Direct Information](https://gdp.cse.chalmers.se/index.php?title=Direct_Information&action=edit&redlink=1 "Direct Information (page does not exist)") are affected by Save-Load Cycles as players can use the information provided as [Trans-Game Information](https://gdp.cse.chalmers.se/index.php?title=Trans-Game_Information&action=edit&redlink=1 "Trans-Game Information (page does not exist)") to make use of it earlier than they were supposed to have knowledge of it. The same applies for information provided by [Near Miss Indicators](https://gdp.cse.chalmers.se/index.php?title=Near_Miss_Indicators "Near Miss Indicators"). This can be seen as a form of [Game Time Manipulation](https://gdp.cse.chalmers.se/index.php?title=Game_Time_Manipulation "Game Time Manipulation") done through non-gameplay actions and allowing for local [Replayability](https://gdp.cse.chalmers.se/index.php?title=Replayability "Replayability") inside game instances. Extensive use of these possibility provided by Save-Load Cycles can lead to [Save Scumming](https://gdp.cse.chalmers.se/index.php?title=Save_Scumming "Save Scumming") (which can be especially likely in games with [Mini Games](https://gdp.cse.chalmers.se/index.php?title=Mini_Games "Mini Games")).

While the pattern is most often used in [Single-Player Games](https://gdp.cse.chalmers.se/index.php?title=Single-Player_Games "Single-Player Games"), in these specifically Save-Load Cycles can encourage player to do [Experimenting](https://gdp.cse.chalmers.se/index.php?title=Experimenting "Experimenting") since the cost of testing something may at most be some [Downtime](https://gdp.cse.chalmers.se/index.php?title=Downtime "Downtime"). The ability to instantly mitigate bad effects of gameplay actions which Save-Load Cycles provide can let players have [Smooth Learning Curves](https://gdp.cse.chalmers.se/index.php?title=Smooth_Learning_Curves&action=edit&redlink=1 "Smooth Learning Curves (page does not exist)") when trying to learn something about a game.

## Relations

### Can Instantiate

[Downtime](https://gdp.cse.chalmers.se/index.php?title=Downtime "Downtime"), [Extra Chances](https://gdp.cse.chalmers.se/index.php?title=Extra_Chances "Extra Chances"), [Freedom of Choice](https://gdp.cse.chalmers.se/index.php?title=Freedom_of_Choice "Freedom of Choice"), [Fudged Results](https://gdp.cse.chalmers.se/index.php?title=Fudged_Results "Fudged Results"), [Game Time Manipulation](https://gdp.cse.chalmers.se/index.php?title=Game_Time_Manipulation "Game Time Manipulation"), [Game Pauses](https://gdp.cse.chalmers.se/index.php?title=Game_Pauses "Game Pauses"), [Interruptibility](https://gdp.cse.chalmers.se/index.php?title=Interruptibility "Interruptibility"), [Negotiable Play Sessions](https://gdp.cse.chalmers.se/index.php?title=Negotiable_Play_Sessions "Negotiable Play Sessions"), [Replayability](https://gdp.cse.chalmers.se/index.php?title=Replayability "Replayability"), [Reversibility](https://gdp.cse.chalmers.se/index.php?title=Reversibility&action=edit&redlink=1 "Reversibility (page does not exist)"), [Save Scumming](https://gdp.cse.chalmers.se/index.php?title=Save_Scumming "Save Scumming"), [Smooth Learning Curves](https://gdp.cse.chalmers.se/index.php?title=Smooth_Learning_Curves&action=edit&redlink=1 "Smooth Learning Curves (page does not exist)"), [Stimulated Planning](https://gdp.cse.chalmers.se/index.php?title=Stimulated_Planning "Stimulated Planning"), [Trans-Game Information](https://gdp.cse.chalmers.se/index.php?title=Trans-Game_Information&action=edit&redlink=1 "Trans-Game Information (page does not exist)")

#### with [Single-Player Games](https://gdp.cse.chalmers.se/index.php?title=Single-Player_Games "Single-Player Games")

[Experimenting](https://gdp.cse.chalmers.se/index.php?title=Experimenting "Experimenting")

### Can Modulate

[Real-Time Games](https://gdp.cse.chalmers.se/index.php?title=Real-Time_Games "Real-Time Games")

### Can Be Instantiated By

[Dedicated Game Facilitators](https://gdp.cse.chalmers.se/index.php?title=Dedicated_Game_Facilitators "Dedicated Game Facilitators"), [Extra-Game Actions](https://gdp.cse.chalmers.se/index.php?title=Extra-Game_Actions "Extra-Game Actions"), [Save Files](https://gdp.cse.chalmers.se/index.php?title=Save_Files "Save Files")

### Can Be Modulated By

[Direct Information](https://gdp.cse.chalmers.se/index.php?title=Direct_Information&action=edit&redlink=1 "Direct Information (page does not exist)"), [Near Miss Indicators](https://gdp.cse.chalmers.se/index.php?title=Near_Miss_Indicators "Near Miss Indicators"), [Save Points](https://gdp.cse.chalmers.se/index.php?title=Save_Points "Save Points"), [Secondary Interface Screens](https://gdp.cse.chalmers.se/index.php?title=Secondary_Interface_Screens "Secondary Interface Screens")

### Possible Closure Effects

\-

### Potentially Conflicting With

[Death Consequences](https://gdp.cse.chalmers.se/index.php?title=Death_Consequences "Death Consequences"), [Difficulty Levels](https://gdp.cse.chalmers.se/index.php?title=Difficulty_Levels "Difficulty Levels"), [Irreversible Events](https://gdp.cse.chalmers.se/index.php?title=Irreversible_Events "Irreversible Events"), [Leaps of Faith](https://gdp.cse.chalmers.se/index.php?title=Leaps_of_Faith "Leaps of Faith"), [Penalties](https://gdp.cse.chalmers.se/index.php?title=Penalties "Penalties"), [Permadeath](https://gdp.cse.chalmers.se/index.php?title=Permadeath "Permadeath"), [Multiplayer Games](https://gdp.cse.chalmers.se/index.php?title=Multiplayer_Games "Multiplayer Games"), [Scores](https://gdp.cse.chalmers.se/index.php?title=Scores "Scores"), [Surprises](https://gdp.cse.chalmers.se/index.php?title=Surprises&action=edit&redlink=1 "Surprises (page does not exist)"), [Temporal Consistency](https://gdp.cse.chalmers.se/index.php?title=Temporal_Consistency "Temporal Consistency"), [Tension](https://gdp.cse.chalmers.se/index.php?title=Tension "Tension"), [Time Limited Game Instances](https://gdp.cse.chalmers.se/index.php?title=Time_Limited_Game_Instances "Time Limited Game Instances")

## History

An updated version of the pattern _Save-Load Cycles_ that was part of the original collection in the book _Patterns in Game Design_<ref name="Bjork & Holopainen 2004"/>.

## References

<references> <ref name="Bjork & Holopainen 2004">Björk, S. & Holopainen, J. (2004) _Patterns in Game Design_. Charles River Media. ISBN1-58450-354-8.</ref> </references>

## Acknowledgements

\-
