use std::{error::Error, env};

use tokio::{io::{AsyncReadExt, AsyncWriteExt}, net::TcpListener, io};


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
            let (mut ri,mut wi) = socket.split();
            io::copy(&mut ri, &mut wi).await
        });
    }
}