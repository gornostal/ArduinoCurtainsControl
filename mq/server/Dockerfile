FROM node:8

WORKDIR /var/app

COPY package.json .

RUN yarn install

COPY . .

CMD yarn start
