FROM node:8-alpine

WORKDIR /var/app

COPY package.json .

EXPOSE 8080

RUN yarn install

COPY . .

CMD yarn start
