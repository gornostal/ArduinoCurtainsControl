const request = require("request")
const config = require("./config")

const allowedCommands = ["open", "close", "toggle", "stop", "ping"]

function connect() {
  const options = {
    url: `${config.apiUrl}/long-polling?accessToken=${config.accessToken}`,
    timeout: 31e3
  }
  try {
    request(options, (error, response, body) => {
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
        connect()
      } catch (e) {
        log(`Error occurred while handling request ${e.message}`)
        setTimeout(connect, 3e3)
      }
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

  const url = `${config.arduinoUrl}/${command}`
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

function handle(signal) {
  return function () {
    log(`Received ${signal}`)
    process.exit(1)
  }
}

process.on("SIGINT", handle("SIGINT"))
process.on("SIGTERM", handle("SIGTERM"))
