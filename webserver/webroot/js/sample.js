// Sample DAG data - in a real application, this would come from an API
export const sampleDags = [
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
