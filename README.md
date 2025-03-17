# vending_machine

# Vending Machine with QR Code Scanner

Overview

This project is a smart vending machine that allows users to scan a QR code to make a purchase. The machine processes the request, communicates with a backend server via HTTP, and dispenses a packet upon successful transaction verification.

Features

QR Code Scanning: Uses an external QR code scanner or camera module.

ESP32 Integration: Handles network communication and hardware control.

TFT Display (ILI9125): Displays transaction details and user prompts.

HTTP Communication: Sends scanned QR code data to a backend server for validation.

Backend Integration: Processes payments and sends a response to the ESP32.

Automatic Dispensing: Dispenses the selected item upon transaction success.

Hardware Components

ESP32 (Microcontroller for processing and communication)

TFT ILI9125 (Display for user interaction)

QR Code Scanner (For scanning user codes)

Motor/Actuator (For dispensing items)

Power Supply

Software & Libraries

Arduino IDE / ESP-IDF (For ESP32 programming)

TFT_eSPI (Library for TFT ILI9125 display)

WiFi & HTTPClient (For network communication)

Backend Server (Developed with Node.js/Python/Django/Flask)

System Flow

User scans a QR code at the vending machine.

ESP32 sends the scanned QR data to the backend server via HTTP.

The server validates the payment and responds with a success/failure status.

The ESP32 processes the response:

If success, the machine dispenses the item and updates the display.

If failure, an error message is shown on the TFT screen.

The machine logs the transaction and updates stock levels (if applicable).

Setup Instructions

# 1. Hardware Setup

Connect the TFT ILI9125 display to ESP32.

Integrate the QR code scanner module with ESP32.

Configure motor/actuator for dispensing items.

Ensure proper power supply connections.

# 2. Software Setup

On ESP32:

Install required libraries (TFT_eSPI, WiFi, HTTPClient).

Configure WiFi credentials.

Implement QR code scanning logic.

Send HTTP requests to the backend.

Process responses and control dispensing mechanism.

Backend Configuration:

Set up a server with Flask/Django/Node.js.

Implement API endpoints to receive and validate QR transactions.

Handle database storage for transactions.

Send appropriate responses to ESP32
