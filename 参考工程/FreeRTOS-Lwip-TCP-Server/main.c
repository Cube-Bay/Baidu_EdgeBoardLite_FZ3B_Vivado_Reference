#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "xil_printf.h"
#include "xil_cache.h"

#include "netif/xadapter.h"
#include "lwip/sockets.h"
#include "lwipopts.h"
#include "lwip/init.h"
#include "lwip/inet.h"

#define SERVER_PORT 1145           // 服务器端口
#define BUFFER_SIZE 1024

int client_sockfd = -1;            // 客户端套接字
int listen_sockfd = -1;            // 监听套接字

char send_recv_task_created = 0;
char buffer_r[BUFFER_SIZE];

// 声明任务句柄
TaskHandle_t network_init;
TaskHandle_t tcp_server;
TaskHandle_t tcp_server_recv;
TaskHandle_t tcp_server_send;

// 声明任务函数
void network_init_Task();
void tcp_server_Task();
void tcp_server_recv_Task();
void tcp_server_send_Task();

int main() {
    xil_printf("TCP Server Starting...\r\n");

    // 创建网络初始化任务
    xTaskCreate(network_init_Task,
                "network_init",
                512,
                NULL,
                0,
                &network_init);

    // 启动任务调度器
    vTaskStartScheduler();

    while(1);
}

void network_init_Task() {
    lwip_init();  // 初始化 Lwip

    // 配置网络接口
    struct netif server_netif;
    ip_addr_t ipaddr, netmask, gw;

    unsigned char mac[] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02};
    IP4_ADDR(&ipaddr, 192, 168, 1, 10);     // 服务器IP
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 1, 1);

    // 添加网络接口
    if (!xemac_add(&server_netif, &ipaddr, &netmask, &gw,
                   mac, XPAR_XEMACPS_0_BASEADDR)) {
        xil_printf("Error adding network interface\n\r");
    }

    netif_set_default(&server_netif);
    netif_set_up(&server_netif);
    xil_printf("Network interface ready\n");

    // 创建网络接收线程
    sys_thread_new("xemacif_input_thread",
                   (void(*)(void*))xemacif_input_thread,
                   &server_netif,
                   1024,
                   2);

    // 创建TCP服务器任务
    xTaskCreate(tcp_server_Task,
                "tcp_server",
				4096,
                NULL,
                0,
                &tcp_server);

    // 删除本任务
    vTaskDelete(NULL);
}

void tcp_server_Task() {

	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);

	xil_printf("Starting TCP server...\n");

	// 1. 创建监听套接字
	listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sockfd < 0) {
		xil_printf("Socket creation failed\n");
		vTaskDelete(NULL);
		return;
	}

	// 2. 配置服务器地址
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 监听所有IP
	server_addr.sin_port = htons(SERVER_PORT);

	// 3. 绑定端口
	if (bind(listen_sockfd, (struct sockaddr*)&server_addr,
			 sizeof(server_addr)) < 0) {
		xil_printf("Bind failed\n");
		close(listen_sockfd);
		vTaskDelete(NULL);
		return;
	}

	while(1){

		// 4. 开始监听
		if (listen(listen_sockfd, 1) < 0) {  // 只允许一个连接
			xil_printf("Listen failed\n");
			close(listen_sockfd);
			vTaskDelete(NULL);
			return;
		}

		xil_printf("TCP Server listening on port %d\n", SERVER_PORT);

		// 5. 等待客户端连接
		xil_printf("Waiting for client connection...\n");

		client_sockfd = accept(listen_sockfd,
							   (struct sockaddr*)&client_addr,
							   &client_len);

		if (client_sockfd < 0) {
			xil_printf("Accept failed\n");
		}

		xil_printf("Client connected from %s:%d\n",
				   inet_ntoa(client_addr.sin_addr),
				   ntohs(client_addr.sin_port));

		if(!send_recv_task_created){

			// 创建网络接收任务
			xTaskCreate(tcp_server_recv_Task,
						"recv",
						2048,
						NULL,
						0,
						&tcp_server_recv);
			// 创建网络发送任务
			xTaskCreate(tcp_server_send_Task,
						"send",
						512,
						NULL,
						0,
						&tcp_server_send);
		}
		else{
			vTaskResume(tcp_server_recv);
			vTaskResume(tcp_server_send);
		}

		send_recv_task_created = 1;

		vTaskSuspend(NULL);

	}
}

void tcp_server_recv_Task(){

	int bytes_received;
//	int total_bytes_received = 0;

	while(1){

		bytes_received = recv(client_sockfd, buffer_r, BUFFER_SIZE , 0);

		if (bytes_received <= 0) {
			if (bytes_received == 0) {
				xil_printf("Client disconnected\n");
			} else {
				xil_printf("Receive error\n");
			}
			// 7. 关闭客户端连接，等待下一个连接
			close(client_sockfd);
			client_sockfd = -1;
			vTaskResume(tcp_server);
			vTaskSuspend(NULL);
			continue;
		}

		printf("Received %d Bytes!\n" , bytes_received);

//		// 数据没收够
//		if(total_bytes_received < BUFFER_SIZE){
//
//			bytes_received = recv(client_sockfd, buffer_r+total_bytes_received, 1000 , 0);
//
//			if (bytes_received <= 0) {
//				if (bytes_received == 0) {
//					xil_printf("Client disconnected\n");
//				} else {
//					xil_printf("Receive error\n");
//				}
//				// 7. 关闭客户端连接，等待下一个连接
//				close(client_sockfd);
//				client_sockfd = -1;
//				vTaskResume(tcp_server);
//				vTaskSuspend(NULL);
//				continue;
//			}
//
//			total_bytes_received = total_bytes_received + bytes_received;
//
//			printf("Received %d Bytes!\n" , total_bytes_received);
//		}
		// 数据收够了
//		else{
//			total_bytes_received = 0;
//		}

	}
}

void tcp_server_send_Task(){

	char Data[] = "TCP Server Send Test";

	while(1){
		write(client_sockfd , Data , sizeof(Data));
		sleep(5);
	}
}
