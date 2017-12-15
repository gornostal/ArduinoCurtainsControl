FROM node:8-alpine

WORKDIR /var/app

COPY package.json .

RUN yarn install

COPY . .

CMD yarn start
