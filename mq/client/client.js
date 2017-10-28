const WebSocket = require("ws")
const request = require("request")

const apiUrl = process.env.API_URL
const accessToken = process.env.ACCESS_TOKEN
const arduinoUrl = process.env.ARDUINO_URL
if (!apiUrl) {
  throw new Error("API_URL env var must be set")
}
if (!accessToken) {
  throw new Error("ACCESS_TOKEN env var must be set")
}
if (!arduinoUrl) {
  throw new Error("ARDUINO_URL env var must be set")
}

const allowedCommands = ["open", "close", "toggle", "stop"]

function connect() {
  log(`Connecting to ${apiUrl}`)
  const ws = new WebSocket(`${apiUrl}/subscribe?accessToken=${accessToken}`)
  ws.on("open", () => log("Connected"))
  ws.on("message", data => handleMessage(JSON.parse(data)))
  ws.on("error", data => log("Error", JSON.stringify(data)))
  ws.on("close", () => {
    // reconnect
    log("Disconnected. Trying to reconnect...")
    setTimeout(connect, 3e3)
  })
}

function handleMessage(data) {
  const { command } = data
  if (!allowedCommands.includes(command)) {
    log(`Unrecognized command "${command}"`)
    return
  }

  const url = `${arduinoUrl}/${command}`
  log(`Requesting GET ${url}`)
  request(url, (error, response, body) => {
    if (error) {
      log(`HTTP error: ${error}`)
    }

    const status = response && response.statusCode
    log(`Response ${status} ${body}`)
  })
}

function log(...args) {
  console.log(new Date(), ...args)
}

connect()
