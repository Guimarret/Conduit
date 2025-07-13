"use client"

import { useState, useEffect } from "react"
import { Plus, Calendar, Clock, Play, Pause, Settings, MoreHorizontal, Trash2 } from "lucide-react"
import { Button } from "../../components/ui/button"
import { Card, CardContent, CardHeader, CardTitle } from "../../components/ui/card"
import { Badge } from "../../components/ui/badge"
import { Alert, AlertDescription } from "../../components/ui/alert"
import { DropdownMenu, DropdownMenuContent, DropdownMenuItem, DropdownMenuTrigger } from "../../components/ui/dropdown-menu"
import TaskCreationModal from "./task-creation-modal"
import type { Task } from "../types/task"
import EditTaskModal from "./edit-task-modal"
import DeleteConfirmationModal from "./delete-confirmation-modal"
import Notification from "./notification"

export default function TaskDashboard() {
  const [tasks, setTasks] = useState<Task[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [isModalOpen, setIsModalOpen] = useState(false)
  const [editingTask, setEditingTask] = useState<Task | null>(null)
  const [deletingTaskId, setDeletingTaskId] = useState<number | null>(null)
  const [notification, setNotification] = useState<{
    message: string
    type: "success" | "error"
  } | null>(null)

  useEffect(() => {
    fetchTasks()
  }, [])

  const fetchTasks = async () => {
    try {
      setLoading(true)
      const response = await fetch("http://localhost:8080/api/dag_data")
      if (!response.ok) {
        throw new Error("Failed to fetch tasks")
      }
      const data = await response.json()
      setTasks(data.tasks || [])
    } catch (err) {
      setError("Unable to fetch tasks. Please ensure the API server is running.")
      // Mock data for demonstration
      setTasks([
        {
          id: 1,
          name: "data_processing",
          schedule: "0 2 * * *",
          execution: "/scripts/process_data.sh",
          status: "active",
        },
        {
          id: 2,
          name: "backup_database",
          schedule: "0 0 * * 0",
          execution: "/scripts/backup.sh",
          status: "active",
        },
        {
          id: 3,
          name: "send_reports",
          schedule: "0 9 * * 1-5",
          execution: "/scripts/reports.py",
          status: "paused",
        },
        {
          id: 4,
          name: "cleanup_logs",
          schedule: "0 1 * * *",
          execution: "/scripts/cleanup.sh",
          status: "active",
        },
        {
          id: 5,
          name: "sync_data",
          schedule: "*/15 * * * *",
          execution: "/scripts/sync.py",
          status: "active",
        },
        {
          id: 6,
          name: "health_check",
          schedule: "*/5 * * * *",
          execution: "/scripts/health.sh",
          status: "active",
        },
      ])
    } finally {
      setLoading(false)
    }
  }

  const formatCronExpression = (cron: string) => {
    const cronDescriptions: { [key: string]: string } = {
      "* * * * *": "Every minute",
      "*/5 * * * *": "Every 5 minutes",
      "*/15 * * * *": "Every 15 minutes",
      "0 * * * *": "Every hour",
      "0 0 * * *": "Daily at midnight",
      "0 2 * * *": "Daily at 2:00 AM",
      "0 1 * * *": "Daily at 1:00 AM",
      "0 0 * * 0": "Weekly on Sunday",
      "0 9 * * 1-5": "Weekdays at 9:00 AM",
    }
    return cronDescriptions[cron] || cron
  }

  const getStatusColor = (status?: string) => {
    switch (status) {
      case "active":
        return "bg-emerald-100 text-emerald-800 border-emerald-200"
      case "paused":
        return "bg-amber-100 text-amber-800 border-amber-200"
      case "failed":
        return "bg-red-100 text-red-800 border-red-200"
      default:
        return "bg-sky-100 text-sky-800 border-sky-200"
    }
  }

  const handleTaskAction = (taskId: number, action: string) => {
    const task = tasks.find((t) => t.id === taskId)

    switch (action) {
      case "edit":
        if (task) {
          setEditingTask(task)
        }
        break
      case "delete":
        setDeletingTaskId(taskId)
        break
      case "run":
      case "pause":
        console.log(`${action} task ${taskId}`)
        break
    }
  }

  const showNotification = (message: string, type: "success" | "error") => {
    setNotification({ message, type })
    setTimeout(() => setNotification(null), 3000)
  }

  if (loading) {
    return (
      <div className="space-y-6">
        <div className="flex justify-between items-center">
          <div className="h-8 w-48 bg-slate-200 rounded animate-pulse" />
          <div className="h-10 w-32 bg-slate-200 rounded animate-pulse" />
        </div>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-4">
          {Array.from({ length: 8 }).map((_, i) => (
            <Card key={i} className="animate-pulse">
              <CardContent className="p-4">
                <div className="space-y-3">
                  <div className="h-5 w-32 bg-slate-200 rounded" />
                  <div className="h-4 w-24 bg-slate-200 rounded" />
                  <div className="h-4 w-36 bg-slate-200 rounded" />
                </div>
              </CardContent>
            </Card>
          ))}
        </div>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      {error && (
        <Alert className="border-amber-200 bg-amber-50">
          <AlertDescription className="text-amber-800">{error}</AlertDescription>
        </Alert>
      )}

      <div className="flex justify-between items-center">
        <div className="flex items-center gap-4">
          <h2 className="text-2xl font-semibold text-slate-800">Tasks Overview</h2>
          <Badge variant="outline" className="bg-sky-50 text-sky-700 border-sky-200">
            {tasks.length} tasks
          </Badge>
        </div>
        <Button onClick={() => setIsModalOpen(true)} className="bg-sky-600 hover:bg-sky-700 text-white">
          <Plus className="w-4 h-4 mr-2" />
          Create Task
        </Button>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 2xl:grid-cols-5 gap-4">
        {tasks.map((task) => (
          <Card key={task.id} className="border-sky-100 hover:shadow-md transition-shadow">
            <CardHeader className="pb-3">
              <div className="flex justify-between items-start">
                <div className="flex-1 min-w-0">
                  <CardTitle className="text-slate-800 text-base truncate">{task.name}</CardTitle>
                  <Badge className={`${getStatusColor(task.status)} mt-2 text-xs`}>{task.status || "active"}</Badge>
                </div>
                <DropdownMenu>
                  <DropdownMenuTrigger asChild>
                    <Button variant="ghost" size="sm" className="text-slate-600 hover:text-sky-600 h-8 w-8 p-0">
                      <MoreHorizontal className="w-4 h-4" />
                    </Button>
                  </DropdownMenuTrigger>
                  <DropdownMenuContent align="end">
                    <DropdownMenuItem onClick={() => handleTaskAction(task.id, "run")}>
                      <Play className="w-4 h-4 mr-2" />
                      Run Now
                    </DropdownMenuItem>
                    <DropdownMenuItem onClick={() => handleTaskAction(task.id, "pause")}>
                      <Pause className="w-4 h-4 mr-2" />
                      Pause
                    </DropdownMenuItem>
                    <DropdownMenuItem onClick={() => handleTaskAction(task.id, "edit")}>
                      <Settings className="w-4 h-4 mr-2" />
                      Edit
                    </DropdownMenuItem>
                    <DropdownMenuItem
                      onClick={() => handleTaskAction(task.id, "delete")}
                      className="text-red-600 focus:text-red-600"
                    >
                      <Trash2 className="w-4 h-4 mr-2" />
                      Delete
                    </DropdownMenuItem>
                  </DropdownMenuContent>
                </DropdownMenu>
              </div>
            </CardHeader>
            <CardContent className="pt-0 space-y-3">
              <div className="space-y-2">
                <div className="flex items-center gap-2 text-xs text-slate-600">
                  <Calendar className="w-3 h-3" />
                  <span className="truncate">{formatCronExpression(task.schedule)}</span>
                </div>
                <div className="flex items-center gap-2 text-xs text-slate-600">
                  <Clock className="w-3 h-3" />
                  <code className="bg-slate-100 px-1 py-0.5 rounded text-xs truncate flex-1">{task.schedule}</code>
                </div>
              </div>
              <div className="text-xs text-slate-600">
                <span className="font-medium">Exec:</span>
                <code className="block bg-slate-100 px-2 py-1 rounded text-xs mt-1 truncate">{task.execution}</code>
              </div>
            </CardContent>
          </Card>
        ))}
      </div>

      {tasks.length === 0 && !loading && (
        <Card className="border-dashed border-2 border-sky-200">
          <CardContent className="flex flex-col items-center justify-center py-12">
            <div className="text-slate-400 mb-4">
              <Calendar className="w-12 h-12" />
            </div>
            <h3 className="text-lg font-medium text-slate-600 mb-2">No tasks found</h3>
            <p className="text-slate-500 text-center mb-4">Get started by creating your first scheduled task</p>
            <Button onClick={() => setIsModalOpen(true)} className="bg-sky-600 hover:bg-sky-700 text-white">
              <Plus className="w-4 h-4 mr-2" />
              Create Your First Task
            </Button>
          </CardContent>
        </Card>
      )}

      <TaskCreationModal isOpen={isModalOpen} onClose={() => setIsModalOpen(false)} onTaskCreated={fetchTasks} />

      <EditTaskModal
        task={editingTask}
        isOpen={!!editingTask}
        onClose={() => setEditingTask(null)}
        onTaskUpdated={fetchTasks}
        onNotification={showNotification}
      />

      <DeleteConfirmationModal
        taskId={deletingTaskId}
        isOpen={!!deletingTaskId}
        onClose={() => setDeletingTaskId(null)}
        onTaskDeleted={fetchTasks}
        onNotification={showNotification}
      />

      {notification && (
        <Notification message={notification.message} type={notification.type} onClose={() => setNotification(null)} />
      )}
    </div>
  )
}
