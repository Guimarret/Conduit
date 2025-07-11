import { type NextRequest, NextResponse } from "next/server"

export async function POST(request: NextRequest) {
  try {
    const formData = await request.formData()
    const tasksJson = formData.get("tasks") as string

    if (!tasksJson) {
      return NextResponse.json({ error: "No tasks provided" }, { status: 400 })
    }

    const tasks = JSON.parse(tasksJson)

    // Log uploaded files
    const files: File[] = []
    for (const [key, value] of formData.entries()) {
      if (key.startsWith("file_") && value instanceof File) {
        files.push(value)
      }
    }

    console.log("Received tasks:", tasks)
    console.log(
      "Received files:",
      files.map((f) => f.name),
    )

    // Here you would typically:
    // 1. Validate the task definitions
    // 2. Store files to a file system or cloud storage
    // 3. Save task definitions to your database
    // 4. Schedule the tasks with your task scheduler

    // For now, we'll simulate a successful response
    const response = {
      success: true,
      message: `Successfully created ${tasks.length} task(s)`,
      tasks: tasks.map((task: any, index: number) => ({
        id: Date.now() + index,
        ...task,
        status: "active",
        created_at: new Date().toISOString(),
      })),
      files: files.map((file) => ({
        name: file.name,
        size: file.size,
        type: file.type,
      })),
    }

    return NextResponse.json(response, { status: 201 })
  } catch (error) {
    console.error("Error creating tasks:", error)
    return NextResponse.json({ error: "Failed to create tasks" }, { status: 500 })
  }
}
