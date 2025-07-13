import { type NextRequest, NextResponse } from "next/server"

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const { id, taskName, cronExpression, taskExecution } = body

    // Validate required fields
    if (!id) {
      return NextResponse.json({ error: "Task ID is required" }, { status: 400 })
    }

    if (!taskName?.trim()) {
      return NextResponse.json({ error: "Task name is required" }, { status: 400 })
    }

    if (!cronExpression?.trim()) {
      return NextResponse.json({ error: "Cron expression is required" }, { status: 400 })
    }

    if (!taskExecution?.trim()) {
      return NextResponse.json({ error: "Task execution is required" }, { status: 400 })
    }

    // Validate cron expression format (basic validation)
    const cronParts = cronExpression.trim().split(" ")
    if (cronParts.length !== 5) {
      return NextResponse.json(
        {
          error: "Invalid cron expression format. Expected 5 parts (minute hour day month weekday)",
        },
        { status: 400 },
      )
    }

    console.log("Forwarding request to external backend:", {
      id,
      taskName: taskName.trim(),
      cronExpression: cronExpression.trim(),
      taskExecution: taskExecution.trim(),
    })

    // Forward the request to the external backend
    const backendResponse = await fetch("http://localhost:8080/api/dag_edit", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        id,
        taskName: taskName.trim(),
        cronExpression: cronExpression.trim(),
        taskExecution: taskExecution.trim(),
      }),
    })

    if (!backendResponse.ok) {
      const errorData = await backendResponse.text()
      console.error("Backend error:", errorData)
      return NextResponse.json(
        { error: "Failed to update task on backend server" },
        { status: backendResponse.status }
      )
    }

    const result = await backendResponse.json()
    
    return NextResponse.json(result, { status: 200 })
  } catch (error) {
    console.error("Error updating task:", error)

    if (error instanceof SyntaxError) {
      return NextResponse.json({ error: "Invalid JSON payload" }, { status: 400 })
    }

    return NextResponse.json(
      {
        error: "Internal server error while updating task",
      },
      { status: 500 },
    )
  }
}
