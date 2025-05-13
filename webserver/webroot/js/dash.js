// Function to render DAG cards
function renderDags(dags) {
  const container = document.getElementById("dags-container");
  container.innerHTML = "";

  // If no dags or empty array, show a message
  if (!dags || dags.length === 0) {
    container.innerHTML = "<p>No DAGs available</p>";
    return;
  }

  dags.forEach((dag) => {
    const card = document.createElement("div");
    card.className = "dag-card";
    card.innerHTML = `
        <div class="dag-header">
            <h3 class="dag-title">${dag.name}</h3>
            <span class="dag-status status-active">active</span>
        </div>
        <div class="dag-info">
            <div>Schedule: ${dag.schedule}</div>
            <div>Last Run: ${dag.execution || "N/A"}</div>
        </div>
        <div class="dag-elements">
            <div class="element-item">Task ID: ${dag.id}</div>
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
  let dagsData = [];
  const fetchAndRenderDags = async () => {
    try {
      const response = await fetch("http://localhost:8080/api/dag_data");
      const responseData = await response.json();
      dagsData = responseData.tasks || [];
      renderDags(dagsData);
    } catch (error) {
      console.error("Error fetching DAG data:", error);
      const container = document.getElementById("dags-container");
      container.innerHTML =
        "<p>Error loading DAGs data. Please try again later.</p>";
    }
  };

  fetchAndRenderDags();

  document.getElementById("search-dags").addEventListener("input", (e) => {
    const searchTerm = e.target.value.toLowerCase();
    const filteredDags = dagsData.filter(
      (dag) => dag.name && dag.name.toLowerCase().includes(searchTerm),
    );
    renderDags(filteredDags);
  });

  const modal = document.getElementById("dag-form-modal");
  const addDagBtn = document.getElementById("add-dag-btn");
  const closeModalBtn = document.querySelector(".close-modal");
  const cancelBtn = document.querySelector(".btn-cancel");
  const addDagForm = document.getElementById("add-dag-form");

  addDagBtn.addEventListener("click", () => {
    modal.style.display = "flex";
  });
  const closeModal = () => {
    modal.style.display = "none";
    addDagForm.reset();
  };

  closeModalBtn.addEventListener("click", closeModal);
  cancelBtn.addEventListener("click", closeModal);
  modal.addEventListener("click", (event) => {
    if (event.target === modal) {
      closeModal();
    }
  });

  addDagForm.addEventListener("submit", (e) => {
    e.preventDefault();

    const formData = new FormData(addDagForm);
    const dagData = {
      name: formData.get("name"),
      cron_job: formData.get("cron_job"),
      second_name: formData.get("binary_name"),
    };

    console.log("New DAG data:", dagData);

    // Here you would typically send this data to your backend API
    // For now, just close the modal
    closeModal();
  });
});
