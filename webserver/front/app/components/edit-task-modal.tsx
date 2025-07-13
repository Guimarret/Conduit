"use client"

import type React from "react"

import { useState, useEffect } from "react"
import { X, Save, Loader2 } from "lucide-react"
import { Button } from "../../components/ui/button"
import { Input } from "../../components/ui/input"
import { Label } from "../../components/ui/label"
import { Card, CardContent, CardHeader, CardTitle } from "../../components/ui/card"
import { Alert, AlertDescription } from "../../components/ui/alert"
import { Badge } from "../../components/ui/badge"
import type { Task } from "../types/task"

interface EditTaskModalProps {
  task: Task | null
  isOpen: boolean
  onClose: () => void
  onTaskUpdated: () => void
  onNotification: (message: string, type: "success" | "error") => void
}

export default function EditTaskModal({ task, isOpen, onClose, onTaskUpdated, onNotification }: EditTaskModalProps) {
  const [formData, setFormData] = useState({
    taskName: "",
    cronExpression: "",
    taskExecution: "",
  })
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    if (task) {
      setFormData({
        taskName: task.name,
        cronExpression: task.schedule,
        taskExecution: task.execution,
      })
      setError(null)
    }
  }, [task])

  if (!isOpen || !task) return null

  const handleInputChange = (field: string, value: string) => {
    setFormData((prev) => ({ ...prev, [field]: value }))
  }

  const validateForm = () => {
    if (!formData.taskName.trim()) {
      throw new Error("Task name is required")
    }
    if (!formData.cronExpression.trim()) {
      throw new Error("Cron expression is required")
    }
    if (!formData.taskExecution.trim()) {
      throw new Error("Task execution is required")
    }
  }

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()

    try {
      setError(null)
      setIsSubmitting(true)

      validateForm()

      const payload = {
        id: task.id,
        taskName: formData.taskName,
        cronExpression: formData.cronExpression,
        taskExecution: formData.taskExecution,
      }

      const response = await fetch("/api/dag_edit", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(payload),
      })

      if (!response.ok) {
        const errorData = await response.json()
        throw new Error(errorData.error || `Failed to update task: ${response.statusText}`)
      }

      const result = await response.json()
      console.log("Task updated successfully:", result)

      onNotification("Task updated successfully", "success")
      onTaskUpdated()
      onClose()
    } catch (err) {
      const errorMessage = err instanceof Error ? err.message : "An error occurred while updating the task"
      setError(errorMessage)
      onNotification(errorMessage, "error")
    } finally {
      setIsSubmitting(false)
    }
  }

  const commonCronExpressions = [
    { label: "Every minute", value: "* * * * *" },
    { label: "Every 5 min", value: "*/5 * * * *" },
    { label: "Every hour", value: "0 * * * *" },
    { label: "Daily midnight", value: "0 0 * * *" },
    { label: "Weekly Sunday", value: "0 0 * * 0" },
    { label: "Monthly 1st", value: "0 0 1 * *" },
  ]

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white rounded-lg shadow-xl max-w-2xl w-full max-h-[90vh] overflow-y-auto">
        <div className="flex justify-between items-center p-6 border-b border-sky-100">
          <h2 className="text-2xl font-semibold text-slate-800">Edit Task</h2>
          <Button variant="ghost" size="sm" onClick={onClose} disabled={isSubmitting}>
            <X className="w-5 h-5" />
          </Button>
        </div>

        <form onSubmit={handleSubmit} className="p-6 space-y-6">
          {error && (
            <Alert className="border-red-200 bg-red-50">
              <AlertDescription className="text-red-800">{error}</AlertDescription>
            </Alert>
          )}

          <Card className="border-sky-100">
            <CardHeader>
              <CardTitle className="text-lg text-slate-800">Task Details</CardTitle>
            </CardHeader>
            <CardContent className="space-y-4">
              <div>
                <Label htmlFor="taskName">Task Name</Label>
                <Input
                  id="taskName"
                  value={formData.taskName}
                  onChange={(e) => handleInputChange("taskName", e.target.value)}
                  placeholder="e.g., data_processing"
                  className="mt-1"
                  disabled={isSubmitting}
                />
              </div>

              <div>
                <Label htmlFor="taskExecution">Task Execution</Label>
                <Input
                  id="taskExecution"
                  value={formData.taskExecution}
                  onChange={(e) => handleInputChange("taskExecution", e.target.value)}
                  placeholder="e.g., /path/to/script.sh"
                  className="mt-1"
                  disabled={isSubmitting}
                />
              </div>

              <div>
                <Label htmlFor="cronExpression">Cron Expression</Label>
                <Input
                  id="cronExpression"
                  value={formData.cronExpression}
                  onChange={(e) => handleInputChange("cronExpression", e.target.value)}
                  placeholder="e.g., 0 2 * * *"
                  className="mt-1"
                  disabled={isSubmitting}
                />
                <div className="flex flex-wrap gap-2 mt-2">
                  {commonCronExpressions.map((cron) => (
                    <Badge
                      key={cron.value}
                      variant="outline"
                      className={`cursor-pointer hover:bg-sky-50 text-xs transition-colors ${
                        isSubmitting ? "opacity-50 cursor-not-allowed" : ""
                      }`}
                      onClick={() => !isSubmitting && handleInputChange("cronExpression", cron.value)}
                    >
                      {cron.label}
                    </Badge>
                  ))}
                </div>
              </div>
            </CardContent>
          </Card>

          <div className="flex justify-end gap-3 pt-4 border-t border-sky-100">
            <Button type="button" variant="outline" onClick={onClose} disabled={isSubmitting}>
              Cancel
            </Button>
            <Button type="submit" className="bg-sky-600 hover:bg-sky-700 text-white" disabled={isSubmitting}>
              {isSubmitting ? (
                <>
                  <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                  Updating...
                </>
              ) : (
                <>
                  <Save className="w-4 h-4 mr-2" />
                  Update Task
                </>
              )}
            </Button>
          </div>
        </form>
      </div>
    </div>
  )
}
