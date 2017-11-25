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

const allowedCommands = ["open", "close", "toggle", "stop", "ping"]

function connect() {
  try {
    request(`${apiUrl}/long-polling?accessToken=${accessToken}`, (error, response, body) => {
      if (error) {
        log(`HTTP error: ${error}. Reconnecting...`)
        setTimeout(connect, 3e3)
        return
      }

      if (response.statusCode === 204) {
        connect()
        return
      }

      try {
        const data = JSON.parse(body)
        handleCommand(data.command)
      } catch (e) {
        log(`Error occurred while handling request ${e.message}`)
      }

      connect()
    })
  } catch (e) {
    log(`Request error: ${e}. Reconnecting...`)
    setTimeout(connect, 3e3)
  }
}

function handleCommand(command) {
  if (!allowedCommands.includes(command)) {
    log(`Unrecognized command "${command}"`)
    return
  }

  if (command === "ping") {
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
