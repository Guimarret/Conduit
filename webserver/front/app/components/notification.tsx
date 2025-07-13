"use client"

import { useEffect, useState } from "react"
import { CheckCircle, XCircle, X } from "lucide-react"

interface NotificationProps {
  message: string
  type: "success" | "error"
  onClose: () => void
}

export default function Notification({ message, type, onClose }: NotificationProps) {
  const [isVisible, setIsVisible] = useState(false)

  useEffect(() => {
    // Trigger animation after component mounts
    const timer = setTimeout(() => setIsVisible(true), 100)
    return () => clearTimeout(timer)
  }, [])

  const handleClose = () => {
    setIsVisible(false)
    setTimeout(onClose, 300) // Wait for animation to complete
  }

  const bgColor = type === "success" ? "bg-emerald-50 border-emerald-200" : "bg-red-50 border-red-200"
  const textColor = type === "success" ? "text-emerald-800" : "text-red-800"
  const iconColor = type === "success" ? "text-emerald-600" : "text-red-600"
  const Icon = type === "success" ? CheckCircle : XCircle

  return (
    <div className="fixed top-4 right-4 z-50">
      <div
        className={`
          ${bgColor} ${textColor} border rounded-lg shadow-lg p-4 max-w-sm w-full
          transform transition-all duration-300 ease-in-out
          ${isVisible ? "translate-x-0 opacity-100" : "translate-x-full opacity-0"}
        `}
      >
        <div className="flex items-start gap-3">
          <Icon className={`w-5 h-5 ${iconColor} flex-shrink-0 mt-0.5`} />
          <div className="flex-1">
            <p className="text-sm font-medium">{message}</p>
          </div>
          <button onClick={handleClose} className={`${textColor} hover:opacity-70 transition-opacity flex-shrink-0`}>
            <X className="w-4 h-4" />
          </button>
        </div>
      </div>
    </div>
  )
}
