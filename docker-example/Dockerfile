FROM nikolaik/python-nodejs:python3.8-nodejs18-slim

WORKDIR /usr/app
#prepare packages to install pynode
RUN apt update -qq && apt install make gcc g++ -y

#prepare npm, shared folder is copied 
#prepare pynode
COPY package.json .

RUN npm install @fridgerator/pynode
RUN npm install 

#prepare python requirements
COPY requirements.txt .
RUN pip3 install -r requirements.txt

#prepare project files
COPY ./src .

CMD ["node", "index.js"]





