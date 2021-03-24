const https = require('https')

exports.handler = async function (request, context) {
    if (request.directive.header.namespace === 'Alexa.Discovery' && request.directive.header.name === 'Discover') {
        log("DEBUG:", "Discover request",  JSON.stringify(request));
        handleDiscovery(request, context, "");
    }
    else if (request.directive.header.namespace === 'Alexa.PowerController') {
        if (request.directive.header.name === 'TurnOn' || request.directive.header.name === 'TurnOff') {
            log("DEBUG:", "TurnOn or TurnOff Request", JSON.stringify(request));
            await handlePowerControl(request, context);
        }
    }

    function handleDiscovery(request, context) {
        var payload = {
            "endpoints":
            [
                {
                    "endpointId": "curtains-04",
                    "manufacturerName": "Aleksandr Gornostal",
                    "friendlyName": "Curtains",
                    "description": "Home Curtains Automation",
                    "displayCategories": ["SWITCH"],
                    "capabilities":
                    [
                        {
                          "type": "AlexaInterface",
                          "interface": "Alexa",
                          "version": "3"
                        },
                        {
                            "interface": "Alexa.PowerController",
                            "version": "3",
                            "type": "AlexaInterface",
                            "properties": {
                                "supported": [{
                                    "name": "powerState"
                                }],
                                 "retrievable": false
                            },
                            "semantics": {
                                "actionMappings": [
                                  {
                                    "@type": "ActionsToDirective",
                                    "actions": ["Alexa.Actions.Close"],
                                    "directive": {
                                      "name": "TurnOff",
                                      "payload": {}
                                    }
                                  },
                                  {
                                    "@type": "ActionsToDirective",
                                    "actions": ["Alexa.Actions.Open"],
                                    "directive": {
                                      "name": "TurnOn",
                                      "payload": {}
                                    }
                                  }
                                ],
                                "stateMappings": [
                                  {
                                    "@type": "StatesToValue",
                                    "states": ["Alexa.States.Closed"],
                                    "value": "OFF"
                                  },
                                  {
                                    "@type": "StatesToValue",
                                    "states": ["Alexa.States.Open"],
                                    "value": "ON"
                                  }  
                                ]
                            }
                        }
                    ]
                }
            ]
        };
        var header = request.directive.header;
        header.name = "Discover.Response";
        log("DEBUG", "Discovery Response: ", JSON.stringify({ header: header, payload: payload }));
        context.succeed({ event: { header: header, payload: payload } });
    }

    function log(message, message1, message2) {
        console.log(message + message1 + message2);
    }

    async function handlePowerControl(request, context) {
        // get device ID passed in during discovery
        var requestMethod = request.directive.header.name;
        var responseHeader = request.directive.header;
        responseHeader.namespace = "Alexa";
        responseHeader.name = "Response";
        responseHeader.messageId = responseHeader.messageId + "-R";
        // get user token pass in request
        var requestToken = request.directive.endpoint.scope.token;
        var powerResult;
        

        if (requestMethod === "TurnOn") {
            await runCurtainsCommand('open')
            powerResult = "ON";
        }
        else if (requestMethod === "TurnOff") {
            await runCurtainsCommand('close')
            powerResult = "OFF";
        }
        var contextResult = {
            "properties": [{
                "namespace": "Alexa.PowerController",
                "name": "powerState",
                "value": powerResult,
                "timeOfSample": "2017-09-03T16:20:50.52Z", //retrieve from result.
                "uncertaintyInMilliseconds": 50
            }]
        };
        var response = {
            context: contextResult,
            event: {
                header: responseHeader,
                endpoint: {
                    scope: {
                        type: "BearerToken",
                        token: requestToken
                    },
                    endpointId: "curtains-04"
                },
                payload: {}
            }
        };
        log("DEBUG", "Alexa.PowerController ", JSON.stringify(response));
        context.succeed(response);
    }
};

function runCurtainsCommand(command) {
    const data = JSON.stringify({ command })

    const options = {
      hostname: 'example.com',
      port: 443,
      path: '/command?accessToken=xxx',
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': data.length
      }
    }

    return new Promise((resolve, reject) => {
        const req = https.request(options, res => {
          console.log(`Response status code: ${res.statusCode} at ${new Date().toISOString()}`)
        })
    
        req.on('error', error => {
          console.error('Received an error from curtains:', error)
          reject(error)
        })
    
        req.write(data)
        req.end(resolve)
    })
}
