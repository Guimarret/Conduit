"use client"

import { useEffect, useState } from "react"
import LoginScreen from "./components/login-screen"
import TaskDashboard from "./components/task-dashboard"

export default function Home() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    // Check if user is already authenticated
    const token = localStorage.getItem("authToken")
    if (token) {
      // In a real app, you'd validate the token with your backend
      setIsAuthenticated(true)
    }
    setIsLoading(false)
  }, [])

  const handleLogin = (token: string) => {
    localStorage.setItem("authToken", token)
    setIsAuthenticated(true)
  }

  const handleLogout = () => {
    localStorage.removeItem("authToken")
    setIsAuthenticated(false)
  }

  if (isLoading) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-sky-50 to-blue-50 flex items-center justify-center">
        <div className="animate-spin rounded-full h-32 w-32 border-t-2 border-b-2 border-sky-600"></div>
      </div>
    )
  }

  if (!isAuthenticated) {
    return <LoginScreen onLogin={handleLogin} />
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-sky-50 to-blue-50">
      <div className="container mx-auto px-4 py-8">
        <div className="mb-8 flex justify-between items-center">
          <div className="flex items-center gap-4">
            <img src="/favicon-96x96.png" alt="Task Dashboard" className="w-12 h-12" />
            <div>
              <h1 className="text-4xl font-bold text-slate-800 mb-2">Task Management Dashboard</h1>
              <p className="text-slate-600">Monitor and manage your scheduled tasks</p>
            </div>
          </div>
          <button onClick={handleLogout} className="px-4 py-2 text-slate-600 hover:text-slate-800 transition-colors">
            Logout
          </button>
        </div>

        <TaskDashboard />
      </div>
    </div>
  )
}
