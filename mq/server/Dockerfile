FROM node:16-alpine

WORKDIR /app
COPY package* .
RUN npm install

COPY . .

CMD node server.js
