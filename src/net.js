const net = require('net');

const req = "GET / HTTP/1.0\r\nHost: www.baidu.com\r\nAccept: */*\r\n\r\n";
const res = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Document</title></head><body><p>this is http response</p></body></html>";

const request = () => {
    return new Promise((resolve, reject) => {
        const sock = net.connect({
            host: '61.135.169.125',
            port: 80
        }, () => {
            sock.write(req);
            const bufs = [];
    
            sock.on('data', (chunk) => {
                bufs.push(chunk);
            });
    
            sock.once('end', () => {
                sock.removeAllListeners();
                resolve();
            });
    
            sock.setTimeout(500, () => {
                sock.removeAllListeners();
                sock.destroy();
                resolve();
            });
    
            sock.once('error', (e) => {
                sock.removeAllListeners();
                sock.destroy();
                resolve(e);
            })
        });
    })

}
net.createServer((socket) => {
    socket.on('error', (e) => {
        console.error(e);
    });

    socket.on("close", () => {
        socket.removeAllListeners();
    })

    Promise.all(
        new Array(10).fill(0).map(() => request())
    )
    .finally(() => {
        socket.end(res);
    })
}).listen(3000);