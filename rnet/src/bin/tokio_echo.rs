use std::{error::Error, env};

use tokio::{io::{AsyncReadExt, AsyncWriteExt}, net::TcpListener};


/**
 * 创建tcp监听，循环接收网络连接，并将每个接收到的字符回写到原连接
*/

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>>{
    let addr = env::args()
    .nth(1)
    .unwrap_or_else(|| "127.0.0.1:50001".to_string());

    let listener = TcpListener::bind(&addr).await?;
    println!("echo Listening on: {}", addr);

    loop {
        let (mut socket, _) = listener.accept().await?;
        
        tokio::spawn(async move {
            let mut buf = [0; 1024];

            // in the loop, read data from the socket adn write the data back
            loop{
        
                let n = socket.read(&mut buf)
                .await
                .expect("failed to read data from socket");

                if n == 0 {
                    return;
                }

                socket.write_all(&buf[0..n])
                .await
                .expect("failed to write data to socket");
            }
        });
    }
}