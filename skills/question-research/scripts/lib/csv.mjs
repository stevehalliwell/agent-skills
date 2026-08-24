import { readFile, writeFile } from "node:fs/promises";

export function parseCsv(text) {
  const rows = []; let row = [], field = "", quoted = false;
  for (let index = 0; index < text.length; index += 1) {
    const character = text[index];
    if (quoted) {
      if (character === '"' && text[index + 1] === '"') { field += '"'; index += 1; }
      else if (character === '"') quoted = false;
      else field += character;
    } else if (character === '"') quoted = true;
    else if (character === ",") { row.push(field); field = ""; }
    else if (character === "\n" || character === "\r") {
      if (character === "\r" && text[index + 1] === "\n") index += 1;
      row.push(field); if (row.some(value => value !== "")) rows.push(row); row = []; field = "";
    } else field += character;
  }
  if (field !== "" || row.length) rows.push([...row, field]);
  if (!rows.length) return [];
  const [headers, ...values] = rows;
  return values.map((cells, rowIndex) => Object.fromEntries(headers.map((header, cellIndex) => [header, cells[cellIndex] ?? ""])));
}

const cell = value => {
  const text = String(value ?? "");
  return /[",\n\r]/.test(text) ? `"${text.replaceAll('"', '""')}"` : text;
};

export const toCsv = (records, headers) => `${headers.join(",")}\n${records.map(record => headers.map(header => cell(record[header])).join(",")).join("\n")}\n`;
export const readCsv = async file => parseCsv(await readFile(file, "utf8"));
export const writeCsv = (file, records, headers) => writeFile(file, toCsv(records, headers));

export function requiredOption(args, name) {
  const index = args.indexOf(name);
  const value = index === -1 ? "" : args[index + 1];
  if (!value || value.startsWith("--")) throw new Error(`Missing required ${name} value.`);
  return value;
}

export const hasHelp = args => args.includes("--help") || args.includes("-h");
