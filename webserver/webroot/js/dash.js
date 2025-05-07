import { sampleDags } from "./sample.js";

// Function to render DAG cards
function renderDags(dags) {
  const container = document.getElementById("dags-container");
  container.innerHTML = "";

  dags.forEach((dag) => {
    const card = document.createElement("div");
    card.className = "dag-card";
    card.innerHTML = `
        <div class="dag-header">
            <h3 class="dag-title">${dag.name}</h3>
            <span class="dag-status status-${dag.status}">${dag.status}</span>
        </div>
        <div class="dag-info">
            <div>Schedule: ${dag.schedule}</div>
            <div>Last Run: ${dag.lastRun}</div>
        </div>
        <div class="dag-elements">
            ${dag.elements.map((element) => `<div class="element-item">${element}</div>`).join("")}
        </div>
        <div class="graph-view" id="graph-${dag.id}"></div>
        <div class="dag-actions">
            <button class="btn-view">View Details</button>
            <button class="btn-run">Run Now</button>
        </div>
    `;
    container.appendChild(card);
  });
}

document.addEventListener("DOMContentLoaded", () => {
  renderDags(sampleDags);

  document.getElementById("search-dags").addEventListener("input", (e) => {
    const searchTerm = e.target.value.toLowerCase();
    const filteredDags = sampleDags.filter(
      (dag) =>
        dag.name.toLowerCase().includes(searchTerm) ||
        dag.elements.some((element) =>
          element.toLowerCase().includes(searchTerm),
        ),
    );
    renderDags(filteredDags);
  });
});
