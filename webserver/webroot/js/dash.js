// Sample DAG data - in a real application, this would come from an API
const sampleDags = [
  {
    id: "dag-001",
    name: "ETL Pipeline",
    status: "active",
    schedule: "Daily at 2:00 AM",
    lastRun: "2023-10-15 02:00:00",
    elements: [
      "Extract Data",
      "Transform",
      "Load to Data Warehouse",
      "Validation",
    ],
  },
  {
    id: "dag-002",
    name: "Data Processing",
    status: "paused",
    schedule: "Hourly",
    lastRun: "2023-10-16 13:00:00",
    elements: ["Get Raw Data", "Clean Data", "Process Data", "Generate Report"],
  },
  {
    id: "dag-003",
    name: "Machine Learning Pipeline",
    status: "active",
    schedule: "Weekly on Sunday",
    lastRun: "2023-10-15 00:00:00",
    elements: [
      "Feature Engineering",
      "Model Training",
      "Model Evaluation",
      "Model Deployment",
    ],
  },
  {
    id: "dag-004",
    name: "Backup Process",
    status: "failed",
    schedule: "Daily at 23:00",
    lastRun: "2023-10-16 23:00:00",
    elements: [
      "Database Backup",
      "File System Backup",
      "Upload to Cloud Storage",
      "Cleanup Old Backups",
    ],
  },
];

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
