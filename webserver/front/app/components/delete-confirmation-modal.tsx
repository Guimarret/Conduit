"use client"

import { useState } from "react"
import { AlertTriangle, Trash2, X, Loader2 } from "lucide-react"
import { Button } from "../../components/ui/button"
import { Card, CardContent } from "../../components/ui/card"
import { Alert, AlertDescription } from "../../components/ui/alert"

interface DeleteConfirmationModalProps {
  taskId: number | null
  isOpen: boolean
  onClose: () => void
  onTaskDeleted: () => void
  onNotification: (message: string, type: "success" | "error") => void
}

export default function DeleteConfirmationModal({
  taskId,
  isOpen,
  onClose,
  onTaskDeleted,
  onNotification,
}: DeleteConfirmationModalProps) {
  const [isDeleting, setIsDeleting] = useState(false)
  const [error, setError] = useState<string | null>(null)

  if (!isOpen || !taskId) return null

  const handleDelete = async () => {
    try {
      setError(null)
      setIsDeleting(true)

      const payload = { id: taskId }

      const response = await fetch("/api/dag_delete", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(payload),
      })

      if (!response.ok) {
        const errorData = await response.json()
        throw new Error(errorData.error || `Failed to delete task: ${response.statusText}`)
      }

      const result = await response.json()
      console.log("Task deleted successfully:", result)

      onNotification("Task deleted successfully", "success")
      onTaskDeleted()
      onClose()
    } catch (err) {
      const errorMessage = err instanceof Error ? err.message : "An error occurred while deleting the task"
      setError(errorMessage)
      onNotification(errorMessage, "error")
    } finally {
      setIsDeleting(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white rounded-lg shadow-xl max-w-md w-full">
        <div className="flex justify-between items-center p-6 border-b border-red-100">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 bg-red-100 rounded-full flex items-center justify-center">
              <AlertTriangle className="w-5 h-5 text-red-600" />
            </div>
            <h2 className="text-xl font-semibold text-slate-800">Delete Task</h2>
          </div>
          <Button variant="ghost" size="sm" onClick={onClose} disabled={isDeleting}>
            <X className="w-5 h-5" />
          </Button>
        </div>

        <div className="p-6 space-y-4">
          {error && (
            <Alert className="border-red-200 bg-red-50">
              <AlertDescription className="text-red-800">{error}</AlertDescription>
            </Alert>
          )}

          <Card className="border-red-100 bg-red-50">
            <CardContent className="p-4">
              <div className="flex items-start gap-3">
                <AlertTriangle className="w-5 h-5 text-red-600 mt-0.5 flex-shrink-0" />
                <div>
                  <h3 className="font-medium text-red-800 mb-1">Confirm Deletion</h3>
                  <p className="text-sm text-red-700">
                    Are you sure you want to delete this task? This action cannot be undone and will permanently remove
                    the task from your system.
                  </p>
                </div>
              </div>
            </CardContent>
          </Card>

          <div className="bg-slate-50 p-3 rounded-lg">
            <p className="text-sm text-slate-600">
              <span className="font-medium">Task ID:</span> {taskId}
            </p>
          </div>
        </div>

        <div className="flex justify-end gap-3 p-6 border-t border-slate-100">
          <Button variant="outline" onClick={onClose} disabled={isDeleting}>
            Cancel
          </Button>
          <Button onClick={handleDelete} className="bg-red-600 hover:bg-red-700 text-white" disabled={isDeleting}>
            {isDeleting ? (
              <>
                <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                Deleting...
              </>
            ) : (
              <>
                <Trash2 className="w-4 h-4 mr-2" />
                Delete Task
              </>
            )}
          </Button>
        </div>
      </div>
    </div>
  )
}
