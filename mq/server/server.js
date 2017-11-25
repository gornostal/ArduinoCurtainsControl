const express = require("express")
const http = require("http")
const url = require("url")
const WebSocket = require("ws")
const bodyParser = require("body-parser")
const timeout = require("connect-timeout")

const app = express()
app.use(bodyParser.json())
app.use(timeout("30s"))
app.use(haltOnTimedout)

const allowedCommands = ["open", "close", "toggle", "stop", "ping"]
const accessToken = process.env.ACCESS_TOKEN
let wsConnection, longPollingResponse, longPollingTimer

if (!accessToken) {
  throw new Error("ACCESS_TOKEN env var must be set")
}

app.post("/command", function(req, res) {
  const clientIp = req.connection.remoteAddress
  if (!req.query.accessToken || req.query.accessToken !== accessToken) {
    log(`accessToken is not set or incorrect. IP ${clientIp}`)
    res.status(403).send({ error: "accessToken is not set or incorrect" })
    return
  }

  const { command } = req.body
  if (!allowedCommands.includes(command)) {
    log(`Unknown or empty command. IP ${clientIp}`)
    res.status(400).send({ error: "Unknown or empty command" })
    return
  }

  if (wsConnection) {
    try {
      wsConnection.send(JSON.stringify({ command }))
      res.send({ accepted: true, sent: true })
      log(`Command sent: ${command}`)
    } catch (e) {
      res.status(500).send({ error: e.message })
    }
    return
  }

  if (longPollingResponse) {
    try {
      longPollingResponse.send({ command })
      longPollingResponse = null
      clearTimeout(longPollingTimer)
      log(`Command sent: ${command}`)
    } catch (e) {
      res.status(500).send({ error: e.message })
    }
    res.send({ accepted: true, sent: true })
    return
  }

  res.send({ accepted: true, sent: false })
})

app.get("/long-polling", (req, res) => {
  const clientIp = req.connection.remoteAddress
  if (!req.query.accessToken || req.query.accessToken !== accessToken) {
    log(`accessToken is not set or incorrect. IP ${clientIp}`)
    res.status(403).send({ error: `accessToken is not set or incorrect` })
    return
  }

  longPollingResponse = res
  clearTimeout(longPollingTimer)
  longPollingTimer = setTimeout(() => {
    longPollingResponse.sendStatus(204)
    longPollingResponse = null
  }, 29e3)
})

const server = http.createServer(app)
const wss = new WebSocket.Server({ server, path: "/subscribe" })

wss.on("connection", function connection(ws, req) {
  const clientIp = req.connection.remoteAddress
  const location = url.parse(req.url, true)
  if (!location.query.accessToken || location.query.accessToken !== accessToken) {
    log(`Incorrect access token from client ${clientIp}`)
    ws.close()
    return
  }

  log(`Connected client ${clientIp}`)
  wsConnection = ws

  ws.on("close", () => {
    log("Clear WS connection")
    wsConnection = null
  })
})

server.listen(8080, () => log(`Listening on ${server.address().port}`))

function log(...args) {
  console.log(new Date(), ...args)
}

function haltOnTimedout(req, res, next) {
  if (!req.timedout) next()
}
