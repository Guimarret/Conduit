export interface Task {
  id: number
  name: string
  schedule: string
  execution: string
  status?: "active" | "paused" | "failed"
  lastRun?: string
  nextRun?: string
}

export interface TaskDefinition {
  taskName: string
  cronExpression: string
  taskExecution: string
}

export interface ApiResponse {
  tasks: Task[]
}
