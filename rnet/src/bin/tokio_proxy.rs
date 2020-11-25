use std::{error::Error, env};

use futures::{FutureExt, future::try_join};
use tokio::{io::{self, AsyncWriteExt}, net::{TcpListener, TcpStream}};

/**
 * a proxy that forwards data to another server and forwards that server's
 * response back to clients. 
*/

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let listen_addr = env::args()
    .nth(1)
    .unwrap_or_else(|| "127.0.0.1:50002".to_string());

    let server_addr = env::args()
    .nth(2)
    .unwrap_or_else(|| "127.0.0.1:50001".to_string());

    println!("listening on: {}", listen_addr);
    println!("proxy to: {}", server_addr);

    let listener = TcpListener::bind(listen_addr).await?;
    
    while let Ok((inbound,_)) = listener.accept().await {
        let transfer = transfer(inbound, server_addr.clone())
            .map(|r|{
                if let Err(e) = r {
                    println!("failed to transfer; error = {}", e);
                }
            });

            tokio::spawn(transfer);
    }

    Ok(())
}

async fn transfer(mut inbound: tokio::net::TcpStream, proxy_addr: String) -> Result<(), Box<dyn Error>> {
    let mut outbound = TcpStream::connect(proxy_addr).await?;

    let (mut ri,mut wi) = inbound.split();
    let (mut ro, mut wo) = outbound.split();

    let client_to_server = async {
        io::copy(&mut ri, &mut wo).await?;
        wo.shutdown().await
    };

    let server_to_client = async {
        io::copy(&mut ro, &mut wi).await?;
        wi.shutdown().await
    };

    try_join(client_to_server, server_to_client);

    Ok(())
}