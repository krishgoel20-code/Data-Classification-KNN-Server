FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libasio-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN git clone https://github.com/CrowCpp/Crow.git && \
    cp -r Crow/include/* .

COPY main.cpp .
COPY templates/ ./templates/

RUN g++ -O3 -std=c++17 main.cpp -I. -lpthread -o classification_server

EXPOSE 18080

CMD ["./classification_server"]