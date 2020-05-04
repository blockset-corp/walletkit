package demo

// Process exit
expect fun quit(): Nothing

// Core System storage path
expect val storagePath: String

// Current seconds
expect fun now(): Long

// Device uuid
expect val uids: String
