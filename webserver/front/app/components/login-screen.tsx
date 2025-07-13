"use client"

import type React from "react"

import { useState } from "react"
import { Eye, EyeOff, Lock, User } from "lucide-react"
import { Button } from "../../components/ui/button"
import { Input } from "../../components/ui/input"
import { Label } from "../../components/ui/label"
import { Card, CardContent, CardHeader, CardTitle } from "../../components/ui/card"
import { Alert, AlertDescription } from "../../components/ui/alert"

interface LoginScreenProps {
  onLogin: (token: string) => void
}

export default function LoginScreen({ onLogin }: LoginScreenProps) {
  const [username, setUsername] = useState("")
  const [password, setPassword] = useState("")
  const [showPassword, setShowPassword] = useState(false)
  const [isLoading, setIsLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setIsLoading(true)
    setError(null)

    try {
      // Simulate API call for authentication
      await new Promise((resolve) => setTimeout(resolve, 1000))

      // Demo credentials - in production, this would be a real API call
      if (username === "admin" && password === "password") {
        // Generate a mock JWT token
        const mockToken = btoa(
          JSON.stringify({
            username,
            exp: Date.now() + 24 * 60 * 60 * 1000, // 24 hours
          }),
        )
        onLogin(mockToken)
      } else {
        throw new Error("Invalid credentials")
      }
    } catch (err) {
      setError("Invalid username or password. Try admin/password")
    } finally {
      setIsLoading(false)
    }
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-sky-50 to-blue-50 flex items-center justify-center p-4">
      <div className="w-full max-w-md">
        <div className="text-center mb-8">
          <img src="/favicon-96x96.png" alt="Task Dashboard" className="w-16 h-16 mx-auto mb-4" />
          <h1 className="text-3xl font-bold text-slate-800 mb-2">Task Management</h1>
          <p className="text-slate-600">Sign in to access your dashboard</p>
        </div>

        <Card className="border-sky-100 shadow-lg">
          <CardHeader>
            <CardTitle className="text-center text-slate-800">Welcome Back</CardTitle>
          </CardHeader>
          <CardContent>
            <form onSubmit={handleSubmit} className="space-y-4">
              {error && (
                <Alert className="border-red-200 bg-red-50">
                  <AlertDescription className="text-red-800">{error}</AlertDescription>
                </Alert>
              )}

              <div className="space-y-2">
                <Label htmlFor="username">Username</Label>
                <div className="relative">
                  <User className="absolute left-3 top-1/2 transform -translate-y-1/2 text-slate-400 w-4 h-4" />
                  <Input
                    id="username"
                    type="text"
                    value={username}
                    onChange={(e) => setUsername(e.target.value)}
                    placeholder="Enter your username"
                    className="pl-10"
                    required
                  />
                </div>
              </div>

              <div className="space-y-2">
                <Label htmlFor="password">Password</Label>
                <div className="relative">
                  <Lock className="absolute left-3 top-1/2 transform -translate-y-1/2 text-slate-400 w-4 h-4" />
                  <Input
                    id="password"
                    type={showPassword ? "text" : "password"}
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    placeholder="Enter your password"
                    className="pl-10 pr-10"
                    required
                  />
                  <button
                    type="button"
                    onClick={() => setShowPassword(!showPassword)}
                    className="absolute right-3 top-1/2 transform -translate-y-1/2 text-slate-400 hover:text-slate-600"
                  >
                    {showPassword ? <EyeOff className="w-4 h-4" /> : <Eye className="w-4 h-4" />}
                  </button>
                </div>
              </div>

              <Button type="submit" className="w-full bg-sky-600 hover:bg-sky-700 text-white" disabled={isLoading}>
                {isLoading ? "Signing in..." : "Sign In"}
              </Button>
            </form>

            <div className="mt-6 p-4 bg-sky-50 rounded-lg">
              <p className="text-sm text-sky-800 font-medium mb-2">Demo Credentials:</p>
              <p className="text-sm text-sky-700">
                Username: <code className="bg-white px-1 rounded">admin</code>
              </p>
              <p className="text-sm text-sky-700">
                Password: <code className="bg-white px-1 rounded">password</code>
              </p>
            </div>
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
