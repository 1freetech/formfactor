type EngineeringStatus = "unknown" | "pass" | "fail";

type PartPlacement = {
  id: number;
  kind: string;
  xMilli: number;
  yMilli: number;
  zMilli: number;
};

type Snapshot = {
  parts: PartPlacement[];
  engineeringStatus?: EngineeringStatus;
  engineeringMessage?: string;
};

const sample: Snapshot = {
  parts: [
    { id: 1, kind: "POWER", xMilli: -1200, yMilli: 280, zMilli: 0 },
    { id: 2, kind: "LED", xMilli: 600, yMilli: 280, zMilli: 0 }
  ]
};

function normalize(snapshot: Snapshot): Required<Snapshot> {
  return {
    parts: [...snapshot.parts].sort((a, b) => a.id - b.id),
    engineeringStatus: snapshot.engineeringStatus ?? "unknown",
    engineeringMessage:
      snapshot.engineeringMessage ??
      "No authoritative C++ engineering result was supplied. Status remains UNKNOWN."
  };
}

function addText(parent: HTMLElement, tag: string, text: string): HTMLElement {
  const element = document.createElement(tag);
  element.textContent = text;
  parent.appendChild(element);
  return element;
}

function render(snapshot: Snapshot): void {
  const target = document.querySelector<HTMLDivElement>("#app");
  if (!target) return;

  const normalized = normalize(snapshot);
  target.replaceChildren();

  const main = document.createElement("main");
  target.appendChild(main);
  addText(main, "h1", "FormFactor Inspector");
  addText(
    main,
    "p",
    `Engineering status: ${normalized.engineeringStatus.toUpperCase()}`
  );
  addText(main, "p", normalized.engineeringMessage);
  addText(main, "p", `Components: ${normalized.parts.length}`);

  const table = document.createElement("table");
  main.appendChild(table);
  const header = document.createElement("tr");
  table.appendChild(header);
  for (const title of ["ID", "Kind", "X milli", "Y milli", "Z milli"]) {
    addText(header, "th", title);
  }

  for (const part of normalized.parts) {
    const row = document.createElement("tr");
    table.appendChild(row);
    for (const value of [part.id, part.kind, part.xMilli, part.yMilli, part.zMilli]) {
      addText(row, "td", String(value));
    }
  }

  addText(
    main,
    "p",
    "The web inspector displays engineering evidence. It never creates a PASS result."
  );
}

render(sample);
