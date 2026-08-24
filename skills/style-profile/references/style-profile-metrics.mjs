#!/usr/bin/env node
/**
 * Summarise repeatable style evidence from Markdown files.
 *
 * Usage:
 *   style-profile-metrics.mjs [--output <metrics.json>] [--paragraph-label-max-words <number>] <Markdown files...>
 *
 * JSON is written to stdout unless --output is supplied. Prose measurements
 * exclude frontmatter, headings, blockquotes, URLs, and fenced/indented code.
 * Vocabulary excludes fenced, indented, inline, and code-like snippets, plus
 * conjunctions, pronouns, determiners, prepositions, particles, numeric/mixed tokens, and one-character tokens. Source text, source paths, and complete
 * word-frequency tables are working data and are never included in the JSON
 * result.
 */

import fs from "node:fs";
import path from "node:path";
import winkNLP from "wink-nlp";
import model from "wink-eng-lite-web-model";

const nlp = winkNLP(model);
const { its } = nlp;
const POS_TAGS = ["ADJ", "ADP", "ADV", "AUX", "CCONJ", "DET", "INTJ", "NOUN", "PART", "PRON", "PROPN", "SCONJ", "VERB"];
const VOCABULARY_EXCLUDED_POS = new Set(["ADP", "CCONJ", "DET", "PART", "PRON", "SCONJ"]);

const args = process.argv.slice(2);
let outputPath;
let paragraphLabelMaxWords = 7;
const files = [];
for (let index = 0; index < args.length; index += 1) {
  const arg = args[index];
  if (arg === "--help" || arg === "-h") {
    console.error("Usage: style-profile-metrics.mjs [--output <metrics.json>] [--paragraph-label-max-words <number>] <Markdown files...>");
    process.exit(0);
  }
  if (arg === "--output") {
    outputPath = args[index + 1];
    if (!outputPath || outputPath.startsWith("-")) {
      console.error("--output requires a JSON file path.");
      process.exit(2);
    }
    index += 1;
    continue;
  }
  if (arg === "--paragraph-label-max-words") {
    paragraphLabelMaxWords = Number.parseInt(args[index + 1], 10);
    if (!Number.isInteger(paragraphLabelMaxWords) || paragraphLabelMaxWords < 1) {
      console.error("--paragraph-label-max-words requires a positive integer.");
      process.exit(2);
    }
    index += 1;
    continue;
  }
  if (arg.startsWith("-")) {
    console.error(`Unknown option: ${arg}`);
    process.exit(2);
  }
  files.push(arg);
}

if (files.length === 0) {
  console.error("Usage: style-profile-metrics.mjs [--output <metrics.json>] [--paragraph-label-max-words <number>] <Markdown files...>");
  process.exit(2);
}

const wordsIn = (text) => text.match(/[\p{L}\p{N}]+(?:['’][\p{L}\p{N}]+)*/gu) ?? [];
const numbers = (values) => {
  const sorted = [...values].sort((a, b) => a - b);
  const at = (p) => sorted.length ? sorted[Math.min(sorted.length - 1, Math.floor((sorted.length - 1) * p))] : 0;
  return {
    count: values.length,
    min: sorted[0] ?? 0,
    p25: at(0.25),
    median: at(0.5),
    p75: at(0.75),
    max: sorted.at(-1) ?? 0,
    average: values.length ? Number((values.reduce((sum, value) => sum + value, 0) / values.length).toFixed(2)) : 0
  };
};
const rate = (count, denominator) => Number((count / Math.max(denominator, 1) * 1000).toFixed(2));
const windowedUniqueWordRatio = (words, windowSize = 100) => {
  const ratios = [];
  for (let start = 0; start < words.length; start += windowSize) {
    const window = words.slice(start, start + windowSize);
    if (window.length >= Math.min(windowSize, 50)) ratios.push(new Set(window.map((word) => word.toLocaleLowerCase())).size / window.length);
  }
  return Number((ratios.reduce((sum, value) => sum + value, 0) / Math.max(ratios.length, 1)).toFixed(3));
};
const vocabularyTokens = (text) => {
  const tokens = [];
  nlp.readDoc(text).tokens().each((token) => {
    const value = token.out(its.value);
    const pos = token.out(its.pos);
    if (VOCABULARY_EXCLUDED_POS.has(pos)) return;
    for (const part of value.split(/[-‐‑‒–—]/u)) {
      if (/^\p{L}{2,}(?:['’]\p{L}{2,})*$/u.test(part)) tokens.push(part.toLocaleLowerCase());
    }
  });
  return tokens;
};
const countWords = (text) => {
  const occurrences = {};
  for (const word of vocabularyTokens(text)) occurrences[word] = (occurrences[word] ?? 0) + 1;
  return occurrences;
};
const wordTypeBreakdown = (text) => {
  const counts = Object.fromEntries(POS_TAGS.map((tag) => [tag, 0]));
  let wordCount = 0;
  nlp.readDoc(text).tokens().each((token) => {
    if (token.out(its.type) !== "word") return;
    const tag = token.out(its.pos);
    if (!POS_TAGS.includes(tag)) return;
    counts[tag] += 1;
    wordCount += 1;
  });
  return {
    wordCount,
    counts,
    percentages: Object.fromEntries(POS_TAGS.map((tag) => [tag, Number((counts[tag] / Math.max(wordCount, 1) * 100).toFixed(3))]))
  };
};
const wordTypeMetrics = (text, paragraphTexts, articles = [wordTypeBreakdown(text)]) => {
  const sentences = sentencesIn(text).map(wordTypeBreakdown);
  const paragraphs = paragraphTexts.map(wordTypeBreakdown);
  const percentageSummary = (units) => Object.fromEntries(POS_TAGS.map((tag) => [tag, numbers(units.map((unit) => unit.percentages[tag]))]));
  return {
    tagSet: "Universal POS",
    includedTags: POS_TAGS,
    article: wordTypeBreakdown(text),
    articlePercentages: percentageSummary(articles),
    sentencePercentages: percentageSummary(sentences),
    paragraphPercentages: percentageSummary(paragraphs)
  };
};
const rankedWords = (occurrences, direction, limit = 25) => Object.entries(occurrences)
  .sort(([wordA, countA], [wordB, countB]) => direction === "most"
    ? countB - countA || wordA.localeCompare(wordB)
    : countA - countB || wordA.localeCompare(wordB))
  .slice(0, limit)
  .map(([word, occurrences]) => ({ word, occurrences }));
const mergeOccurrences = (target, source) => {
  for (const [word, count] of Object.entries(source)) target[word] = (target[word] ?? 0) + count;
};

const isCodeLikeLine = (line) => /^\s*(?:\/\/|\/\*|\*\/|[{}]|(?:const|let|var|class|function|if|for|while|return|public|private|protected|static|using|namespace|import|export)\b)/.test(line)
  || (/^\s*[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*\s*(?:=|\()/.test(line) && /[;{}]/.test(line));
const proseFromMarkdown = (markdown) => {
  let text = markdown.replace(/\r\n/g, "\n").replace(/^---\n[\s\S]*?\n---\n?/, "");
  text = text.replace(/^\s*```[\s\S]*?^\s*```\s*$/gm, "");
  text = text.replace(/`[^`\n]*`/g, "").replace(/https?:\/\/\S+/g, "");
  return text.split("\n")
    .filter((line) => !/^\s*(?:#{1,6}\s|>|```|    )/.test(line) && !isCodeLikeLine(line))
    .map((line) => line.replace(/^\s*(?:[-+*•]|\d+[.)])\s+/, ""))
    .join("\n");
};
const sentencesIn = (text) => text.match(/[^.!?\n]+[.!?]+|[^.!?\n]+$/gm) ?? [];
const isStandaloneLabel = (text) => {
  const words = wordsIn(text);
  return words.length <= paragraphLabelMaxWords && sentencesIn(text).length === 1;
};
const paragraphProgression = (paragraphs, totalWords) => [0, 1, 2, 3].map((segment) => {
  const startProgress = segment / 4;
  const endProgress = (segment + 1) / 4;
  const members = paragraphs.filter((paragraph) => {
    const progress = totalWords ? paragraph.startWord / totalWords : 0;
    return segment === 3 ? progress >= startProgress && progress <= endProgress : progress >= startProgress && progress < endProgress;
  });
  const lengthsWords = members.map((paragraph) => paragraph.wordCount);
  const lengthsSentences = members.map((paragraph) => paragraph.sentenceCount);
  return {
    segment: segment + 1,
    startProgress,
    endProgress,
    paragraphCount: members.length,
    wordCount: lengthsWords.reduce((sum, value) => sum + value, 0),
    sentenceCount: lengthsSentences.reduce((sum, value) => sum + value, 0),
    paragraphLengthWords: numbers(lengthsWords),
    paragraphLengthSentences: numbers(lengthsSentences)
  };
});

const documents = files.map((file, documentIndex) => {
  const markdown = fs.readFileSync(file, "utf8");
  const prose = proseFromMarkdown(markdown);
  const paragraphTexts = prose.split(/\n\s*\n/).map((paragraph) => paragraph.trim()).filter(Boolean);
  let startWord = 0;
  const paragraphEntries = paragraphTexts.map((text, index) => {
    const words = wordsIn(text);
    const sentences = sentencesIn(text);
    const paragraph = {
      index: index + 1,
      startWord,
      wordCount: words.length,
      sentenceCount: sentences.length,
      standaloneLabel: isStandaloneLabel(text),
      averageSentenceLengthWords: sentences.length ? Number((words.length / sentences.length).toFixed(2)) : 0
    };
    startWord += words.length;
    return paragraph;
  });
  const paragraphs = paragraphEntries.filter((paragraph) => !paragraph.standaloneLabel);
  const standaloneLabels = paragraphEntries.filter((paragraph) => paragraph.standaloneLabel);
  const allWords = wordsIn(prose);
  const sentences = sentencesIn(prose);
  const wordOccurrences = countWords(prose);
  const wordTypes = wordTypeMetrics(prose, paragraphTexts);
  const paragraphLengthsWords = paragraphs.map((paragraph) => paragraph.wordCount);
  const paragraphLengthsSentences = paragraphs.map((paragraph) => paragraph.sentenceCount);
  const sentenceLengthsWords = sentences.map((sentence) => wordsIn(sentence).length).filter(Boolean);
  const punctuation = (symbol) => rate((prose.match(new RegExp(symbol, "g")) ?? []).length, allWords.length);
  const proseWordCount = allWords.length;

  for (const paragraph of paragraphs) paragraph.progress = proseWordCount ? Number((paragraph.startWord / proseWordCount).toFixed(6)) : 0;

  return {
    document: documentIndex + 1,
    proseWordCount,
    sentenceCount: sentences.length,
    paragraphCount: paragraphs.length,
    wordOccurrences,
    wordTypes,
    raw: { sentenceLengthsWords, paragraphLengthsWords, paragraphLengthsSentences, paragraphs, paragraphTexts, standaloneLabels },
    metrics: {
      averageWordLength: Number((allWords.reduce((sum, word) => sum + [...word].length, 0) / Math.max(proseWordCount, 1)).toFixed(2)),
      uniqueWordRatio: Number((new Set(allWords.map((word) => word.toLocaleLowerCase())).size / Math.max(proseWordCount, 1)).toFixed(3)),
      windowedUniqueWordRatio: windowedUniqueWordRatio(allWords),
      contractionsPer1000Words: rate((prose.match(/\b[\p{L}]+(?:['’][\p{L}]+)+\b/gu) ?? []).length, proseWordCount),
      questionsPer1000Words: rate((prose.match(/\?/g) ?? []).length, proseWordCount),
      fragmentRate: { available: false, reason: "reliable fragment detection requires syntactic analysis" },
      sentenceLengthWords: numbers(sentenceLengthsWords),
      shortSentencesUnder8Words: Number((sentenceLengthsWords.filter((length) => length < 8).length / Math.max(sentenceLengthsWords.length, 1)).toFixed(3)),
      paragraphLengthWords: numbers(paragraphLengthsWords),
      paragraphLengthSentences: numbers(paragraphLengthsSentences),
      paragraphProgression: paragraphProgression(paragraphs, proseWordCount),
      standaloneLabelsPer1000Words: rate(standaloneLabels.length, proseWordCount),
      standaloneLabelLengthWords: numbers(standaloneLabels.map((paragraph) => paragraph.wordCount)),
      listItemsPer1000Words: rate((markdown.match(/^\s*(?:[-+*•]|\d+[.)])\s+/gm) ?? []).length, proseWordCount),
      punctuationPer1000Words: { emDash: punctuation("—"), semicolon: punctuation(";"), colon: punctuation(":"), parentheses: punctuation("\\(") }
    }
  };
});

const corpusProseWordCount = documents.reduce((sum, document) => sum + document.proseWordCount, 0);
const corpusSentences = documents.flatMap((document) => document.raw.sentenceLengthsWords);
const corpusParagraphWords = documents.flatMap((document) => document.raw.paragraphLengthsWords);
const corpusParagraphSentences = documents.flatMap((document) => document.raw.paragraphLengthsSentences);
const corpusStandaloneLabels = documents.flatMap((document) => document.raw.standaloneLabels);
const corpusParagraphProgression = [0, 1, 2, 3].map((segment) => {
  const startProgress = segment / 4;
  const endProgress = (segment + 1) / 4;
  const paragraphs = documents.flatMap((document) => document.raw.paragraphs.filter((paragraph) =>
    segment === 3
      ? paragraph.progress >= startProgress && paragraph.progress <= endProgress
      : paragraph.progress >= startProgress && paragraph.progress < endProgress
  ));
  const lengthsWords = paragraphs.map((paragraph) => paragraph.wordCount);
  const lengthsSentences = paragraphs.map((paragraph) => paragraph.sentenceCount);
  return {
    segment: segment + 1,
    startProgress,
    endProgress,
    paragraphCount: paragraphs.length,
    wordCount: lengthsWords.reduce((sum, value) => sum + value, 0),
    sentenceCount: lengthsSentences.reduce((sum, value) => sum + value, 0),
    paragraphLengthWords: numbers(lengthsWords),
    paragraphLengthSentences: numbers(lengthsSentences)
  };
});
const corpusOccurrences = {};
for (const document of documents) mergeOccurrences(corpusOccurrences, document.wordOccurrences);
const corpusRawProse = files.map((file) => proseFromMarkdown(fs.readFileSync(file, "utf8"))).join("\n\n");
const corpusWords = wordsIn(corpusRawProse);
const corpusWordTypes = wordTypeMetrics(
  corpusRawProse,
  documents.flatMap((document) => document.raw.paragraphTexts),
  documents.map((document) => document.wordTypes.article)
);
const headers = [];
let lists = 0;
let codeBlocks = 0;
for (const file of files) {
  const markdown = fs.readFileSync(file, "utf8").replace(/\r\n/g, "\n");
  headers.push(...[...markdown.matchAll(/^(#{1,6})\s+/gm)].map((match) => match[1].length));
  lists += (markdown.match(/^\s*(?:[-+*•]|\d+[.)])\s+/gm) ?? []).length;
  codeBlocks += Math.floor((markdown.match(/^\s*```/gm) ?? []).length / 2);
}
const corpusPunctuation = (symbol) => rate((corpusRawProse.match(new RegExp(symbol, "g")) ?? []).length, corpusProseWordCount);
const documentSummary = (selector) => numbers(documents.map(selector));
const report = {
  schemaVersion: 6,
  metrics: {
    measurement: {
      standaloneLabelMaxWords: paragraphLabelMaxWords,
      standaloneLabelRule: "A prose block of one sentence and no more than standaloneLabelMaxWords words; Markdown headings are excluded before prose analysis."
    },
    files: files.length,
    proseWordCount: corpusProseWordCount,
    vocabulary: {
      normalisation: "Prose-only words are lowercased. Common and rare vocabulary excludes conjunctions, pronouns, determiners, prepositions, particles, numeric and mixed tokens, and all one-character tokens. Windowed unique-word ratio uses consecutive 100-word windows (with a final window of at least 50 words) to avoid document-length bias.",
      uniqueWordCount: Object.keys(corpusOccurrences).length,
      windowedUniqueWordRatio: windowedUniqueWordRatio(corpusWords),
      mostCommonWords: rankedWords(corpusOccurrences, "most"),
      leastCommonWords: rankedWords(corpusOccurrences, "least")
    },
    wordTypes: corpusWordTypes,
    averageWordLength: Number((corpusWords.reduce((sum, word) => sum + [...word].length, 0) / Math.max(corpusProseWordCount, 1)).toFixed(2)),
    uniqueWordRatio: Number((new Set(corpusWords.map((word) => word.toLocaleLowerCase())).size / Math.max(corpusProseWordCount, 1)).toFixed(3)),
    contractionsPer1000Words: rate((corpusRawProse.match(/\b[\p{L}]+(?:['’][\p{L}]+)+\b/gu) ?? []).length, corpusProseWordCount),
    questionsPer1000Words: rate((corpusRawProse.match(/\?/g) ?? []).length, corpusProseWordCount),
    fragmentRate: { available: false, reason: "reliable fragment detection requires syntactic analysis" },
    sentenceLengthWords: numbers(corpusSentences),
    shortSentencesUnder8Words: Number((corpusSentences.filter((length) => length < 8).length / Math.max(corpusSentences.length, 1)).toFixed(3)),
    paragraphLengthWords: numbers(corpusParagraphWords),
    paragraphLengthSentences: numbers(corpusParagraphSentences),
    paragraphProgression: corpusParagraphProgression,
    standaloneLabelsPer1000Words: rate(corpusStandaloneLabels.length, corpusProseWordCount),
    standaloneLabelLengthWords: numbers(corpusStandaloneLabels.map((paragraph) => paragraph.wordCount)),
    listItemsPer1000Words: rate(lists, corpusProseWordCount),
    headerDepth: Object.fromEntries([...new Set(headers)].sort().map((depth) => [depth, headers.filter((item) => item === depth).length])),
    fencedCodeBlocks: codeBlocks,
    punctuationPer1000Words: { emDash: corpusPunctuation("—"), semicolon: corpusPunctuation(";"), colon: corpusPunctuation(":"), parentheses: corpusPunctuation("\\(") },
    comparisonDistributions: {
      normalisation: "Per-document summaries used by style-profile-compare.mjs. Values are compared to the corpus centre with robust spread, not to corpus-wide totals.",
      averageWordLength: documentSummary((document) => document.metrics.averageWordLength),
      windowedUniqueWordRatio: documentSummary((document) => document.metrics.windowedUniqueWordRatio),
      contractionsPer1000Words: documentSummary((document) => document.metrics.contractionsPer1000Words),
      questionsPer1000Words: documentSummary((document) => document.metrics.questionsPer1000Words),
      shortSentencesUnder8Words: documentSummary((document) => document.metrics.shortSentencesUnder8Words),
      standaloneLabelsPer1000Words: documentSummary((document) => document.metrics.standaloneLabelsPer1000Words),
      listItemsPer1000Words: documentSummary((document) => document.metrics.listItemsPer1000Words),
      punctuationPer1000Words: Object.fromEntries(["emDash", "semicolon", "colon", "parentheses"].map((key) => [key, documentSummary((document) => document.metrics.punctuationPer1000Words[key])]))
    }
  }
};

const json = `${JSON.stringify(report, null, 2)}\n`;
if (outputPath) {
  fs.mkdirSync(path.dirname(outputPath), { recursive: true });
  fs.writeFileSync(outputPath, json);
  console.error(`Wrote ${outputPath}`);
} else {
  process.stdout.write(json);
}
console.error(`${report.metrics.files} files; ${report.metrics.proseWordCount} prose words; sentence median ${report.metrics.sentenceLengthWords.median} words; paragraph median ${report.metrics.paragraphLengthWords.median} words / ${report.metrics.paragraphLengthSentences.median} sentences.`);
