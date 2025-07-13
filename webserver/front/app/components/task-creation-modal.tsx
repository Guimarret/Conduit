"use client"

import type React from "react"

import { useState } from "react"
import { X, Upload, Plus, Trash2, CheckCircle } from "lucide-react"
import { Button } from "../../components/ui/button"
import { Input } from "../../components/ui/input"
import { Label } from "../../components/ui/label"
import { Textarea } from "../../components/ui/textarea"
import { Card, CardContent, CardHeader, CardTitle } from "../../components/ui/card"
import { Alert, AlertDescription } from "../../components/ui/alert"
import { Badge } from "../../components/ui/badge"

interface TaskCreationModalProps {
  isOpen: boolean
  onClose: () => void
  onTaskCreated: () => void
}

interface TaskDefinition {
  taskName: string
  cronExpression: string
  taskExecution: string
}

export default function TaskCreationModal({ isOpen, onClose, onTaskCreated }: TaskCreationModalProps) {
  const [tasks, setTasks] = useState<TaskDefinition[]>([{ taskName: "", cronExpression: "", taskExecution: "" }])
  const [jsonInput, setJsonInput] = useState("")
  const [uploadedFiles, setUploadedFiles] = useState<File[]>([])
  const [inputMode, setInputMode] = useState<"form" | "json">("form")
  const [error, setError] = useState<string | null>(null)
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [success, setSuccess] = useState(false)

  if (!isOpen) return null

  const addTask = () => {
    setTasks([...tasks, { taskName: "", cronExpression: "", taskExecution: "" }])
  }

  const removeTask = (index: number) => {
    if (tasks.length > 1) {
      setTasks(tasks.filter((_, i) => i !== index))
    }
  }

  const updateTask = (index: number, field: keyof TaskDefinition, value: string) => {
    const updatedTasks = [...tasks]
    updatedTasks[index][field] = value
    setTasks(updatedTasks)
  }

  const handleFileUpload = (event: React.ChangeEvent<HTMLInputElement>) => {
    const files = Array.from(event.target.files || [])
    setUploadedFiles([...uploadedFiles, ...files])
  }

  const removeFile = (index: number) => {
    setUploadedFiles(uploadedFiles.filter((_, i) => i !== index))
  }

  const validateTasks = (tasksToValidate: TaskDefinition[]) => {
    for (const task of tasksToValidate) {
      if (!task.taskName.trim()) {
        throw new Error("Task name is required")
      }
      if (!task.cronExpression.trim()) {
        throw new Error("Cron expression is required")
      }
      if (!task.taskExecution.trim()) {
        throw new Error("Task execution is required")
      }
    }
  }

  const handleSubmit = async () => {
    try {
      setError(null)
      setIsSubmitting(true)
      let tasksToSubmit: TaskDefinition[] = []

      if (inputMode === "form") {
        tasksToSubmit = tasks
      } else {
        try {
          tasksToSubmit = JSON.parse(jsonInput)
          if (!Array.isArray(tasksToSubmit)) {
            throw new Error("JSON must be an array of tasks")
          }
        } catch (e) {
          throw new Error("Invalid JSON format")
        }
      }

      validateTasks(tasksToSubmit)

      // Create FormData for file uploads
      const formData = new FormData()
      formData.append("tasks", JSON.stringify(tasksToSubmit))

      uploadedFiles.forEach((file, index) => {
        formData.append(`file_${index}`, file)
      })

      // POST request to create tasks
      const response = await fetch("http://localhost:8080/api/tasks", {
        method: "POST",
        body: formData,
      })

      if (!response.ok) {
        throw new Error(`Failed to create tasks: ${response.statusText}`)
      }

      const result = await response.json()
      console.log("Tasks created successfully:", result)

      setSuccess(true)
      setTimeout(() => {
        onTaskCreated()
        onClose()
        resetForm()
      }, 1500)
    } catch (err) {
      setError(err instanceof Error ? err.message : "An error occurred")
    } finally {
      setIsSubmitting(false)
    }
  }

  const resetForm = () => {
    setTasks([{ taskName: "", cronExpression: "", taskExecution: "" }])
    setJsonInput("")
    setUploadedFiles([])
    setInputMode("form")
    setError(null)
    setSuccess(false)
  }

  const commonCronExpressions = [
    { label: "Every minute", value: "* * * * *" },
    { label: "Every 5 min", value: "*/5 * * * *" },
    { label: "Every hour", value: "0 * * * *" },
    { label: "Daily midnight", value: "0 0 * * *" },
    { label: "Weekly Sunday", value: "0 0 * * 0" },
    { label: "Monthly 1st", value: "0 0 1 * *" },
  ]

  if (success) {
    return (
      <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
        <div className="bg-white rounded-lg shadow-xl max-w-md w-full p-8 text-center">
          <CheckCircle className="w-16 h-16 text-emerald-500 mx-auto mb-4" />
          <h2 className="text-2xl font-semibold text-slate-800 mb-2">Tasks Created Successfully!</h2>
          <p className="text-slate-600">Your tasks have been submitted and will be processed shortly.</p>
        </div>
      </div>
    )
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white rounded-lg shadow-xl max-w-6xl w-full max-h-[90vh] overflow-y-auto">
        <div className="flex justify-between items-center p-6 border-b border-sky-100">
          <h2 className="text-2xl font-semibold text-slate-800">Create New Tasks</h2>
          <Button variant="ghost" size="sm" onClick={onClose}>
            <X className="w-5 h-5" />
          </Button>
        </div>

        <div className="p-6 space-y-6">
          {error && (
            <Alert className="border-red-200 bg-red-50">
              <AlertDescription className="text-red-800">{error}</AlertDescription>
            </Alert>
          )}

          {/* Input Mode Toggle */}
          <div className="flex gap-2">
            <Button
              variant={inputMode === "form" ? "default" : "outline"}
              onClick={() => setInputMode("form")}
              className={inputMode === "form" ? "bg-sky-600 hover:bg-sky-700" : ""}
            >
              Form Input
            </Button>
            <Button
              variant={inputMode === "json" ? "default" : "outline"}
              onClick={() => setInputMode("json")}
              className={inputMode === "json" ? "bg-sky-600 hover:bg-sky-700" : ""}
            >
              JSON Input
            </Button>
          </div>

          {inputMode === "form" ? (
            <div className="space-y-4">
              {tasks.map((task, index) => (
                <Card key={index} className="border-sky-100">
                  <CardHeader className="pb-3">
                    <div className="flex justify-between items-center">
                      <CardTitle className="text-lg text-slate-800">Task {index + 1}</CardTitle>
                      {tasks.length > 1 && (
                        <Button
                          variant="ghost"
                          size="sm"
                          onClick={() => removeTask(index)}
                          className="text-red-600 hover:text-red-700"
                        >
                          <Trash2 className="w-4 h-4" />
                        </Button>
                      )}
                    </div>
                  </CardHeader>
                  <CardContent className="space-y-4">
                    <div className="grid md:grid-cols-2 gap-4">
                      <div>
                        <Label htmlFor={`taskName-${index}`}>Task Name</Label>
                        <Input
                          id={`taskName-${index}`}
                          value={task.taskName}
                          onChange={(e) => updateTask(index, "taskName", e.target.value)}
                          placeholder="e.g., data_processing"
                          className="mt-1"
                        />
                      </div>

                      <div>
                        <Label htmlFor={`taskExecution-${index}`}>Task Execution</Label>
                        <Input
                          id={`taskExecution-${index}`}
                          value={task.taskExecution}
                          onChange={(e) => updateTask(index, "taskExecution", e.target.value)}
                          placeholder="e.g., /path/to/script.sh"
                          className="mt-1"
                        />
                      </div>
                    </div>

                    <div>
                      <Label htmlFor={`cronExpression-${index}`}>Cron Expression</Label>
                      <Input
                        id={`cronExpression-${index}`}
                        value={task.cronExpression}
                        onChange={(e) => updateTask(index, "cronExpression", e.target.value)}
                        placeholder="e.g., 0 2 * * *"
                        className="mt-1"
                      />
                      <div className="flex flex-wrap gap-2 mt-2">
                        {commonCronExpressions.map((cron) => (
                          <Badge
                            key={cron.value}
                            variant="outline"
                            className="cursor-pointer hover:bg-sky-50 text-xs"
                            onClick={() => updateTask(index, "cronExpression", cron.value)}
                          >
                            {cron.label}
                          </Badge>
                        ))}
                      </div>
                    </div>
                  </CardContent>
                </Card>
              ))}

              <Button
                variant="outline"
                onClick={addTask}
                className="w-full border-dashed border-sky-300 text-sky-600 hover:bg-sky-50 bg-transparent"
              >
                <Plus className="w-4 h-4 mr-2" />
                Add Another Task
              </Button>
            </div>
          ) : (
            <div>
              <Label htmlFor="jsonInput">JSON Task Definitions</Label>
              <Textarea
                id="jsonInput"
                value={jsonInput}
                onChange={(e) => setJsonInput(e.target.value)}
                placeholder={`[
  {
    "taskName": "task1",
    "cronExpression": "0 */2 * * *",
    "taskExecution": "/path/to/script1.sh"
  },
  {
    "taskName": "task2",
    "cronExpression": "0 0 * * *",
    "taskExecution": "/path/to/script2.sh"
  }
]`}
                className="mt-1 h-64 font-mono text-sm"
              />
            </div>
          )}

          {/* File Upload Section */}
          <Card className="border-sky-100">
            <CardHeader>
              <CardTitle className="text-lg text-slate-800 flex items-center gap-2">
                <img src="/favicon-96x96.png" alt="Upload" className="w-6 h-6" />
                Binary Executable Upload
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="border-2 border-dashed border-sky-200 rounded-lg p-6 text-center">
                <Upload className="w-8 h-8 text-sky-400 mx-auto mb-2" />
                <div className="space-y-2">
                  <p className="text-sm text-slate-600">Upload binary executable files (optional)</p>
                  <input
                    type="file"
                    onChange={handleFileUpload}
                    className="hidden"
                    id="file-upload"
                    accept=".exe,.bin,.sh,.py"
                    multiple
                  />
                  <Label
                    htmlFor="file-upload"
                    className="inline-flex items-center px-4 py-2 bg-sky-50 text-sky-700 rounded-md cursor-pointer hover:bg-sky-100 transition-colors"
                  >
                    Choose Files
                  </Label>
                </div>
              </div>

              {uploadedFiles.length > 0 && (
                <div className="mt-4 space-y-2">
                  <p className="text-sm font-medium text-slate-700">Uploaded Files:</p>
                  <div className="space-y-2">
                    {uploadedFiles.map((file, index) => (
                      <div key={index} className="flex items-center justify-between bg-slate-50 p-2 rounded">
                        <span className="text-sm text-slate-700 truncate">{file.name}</span>
                        <Button
                          variant="ghost"
                          size="sm"
                          onClick={() => removeFile(index)}
                          className="text-red-600 hover:text-red-700 h-6 w-6 p-0"
                        >
                          <X className="w-4 h-4" />
                        </Button>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </CardContent>
          </Card>
        </div>

        <div className="flex justify-end gap-3 p-6 border-t border-sky-100">
          <Button variant="outline" onClick={onClose} disabled={isSubmitting}>
            Cancel
          </Button>
          <Button onClick={handleSubmit} className="bg-sky-600 hover:bg-sky-700 text-white" disabled={isSubmitting}>
            {isSubmitting ? "Creating Tasks..." : "Create Tasks"}
          </Button>
        </div>
      </div>
    </div>
  )
}
