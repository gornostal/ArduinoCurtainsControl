const express = require("express")
const http = require("http")
const url = require("url")
const WebSocket = require("ws")
const bodyParser = require("body-parser")

const app = express()
app.use(bodyParser.json())

const allowedCommands = ["open", "close", "toggle", "stop"]
const accessToken = process.env.ACCESS_TOKEN
let wsConnection

if (!accessToken) {
  throw new Error("ACCESS_TOKEN env var must be set")
}

app.post("/command", function(req, res) {
  if (!req.query.accessToken || req.query.accessToken !== accessToken) {
    res.status(403).send({ error: "accessToken is not set or incorrect" })
    return
  }

  if (!allowedCommands.includes(req.body.command)) {
    res.status(400).send({ error: "Unknown or empty command" })
    return
  }

  if (!wsConnection) {
    res.send({ accepted: true, sent: false })
    return
  }

  try {
    wsConnection.send(JSON.stringify({ command: req.body.command }))
    res.send({ accepted: true, sent: true })
  } catch (e) {
    res.status(500).send({ error: e.message })
  }
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
