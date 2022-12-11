package kafka;

import java.util.Collections;
import java.util.Properties;

import javax.swing.tree.ExpandVetoException;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.lang.*;
import java.util.*;

import org.apache.kafka.clients.admin.*;
import org.apache.kafka.clients.admin.AdminClient;
import org.apache.kafka.clients.admin.AdminClientConfig;
import org.apache.kafka.clients.admin.CreateTopicsResult;
import org.apache.kafka.clients.admin.NewTopic;
import org.apache.kafka.common.config.TopicConfig;
import org.apache.kafka.common.KafkaFuture;


public class Client {
	static String ID;
	static String intro = "SogangTalk> ";
	static Consumer cons;
	static Producer prod;

	public static void screen_1() throws IOException { 
		try {
			boolean quit = false;
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			while (!quit) {
				System.out.printf("===============================\n");
				System.out.printf("Welcome to SogangTalk\n");
				System.out.printf("1. Log in\n");
				System.out.printf("2. Exit\n");
				System.out.printf("===============================\n");

				System.out.printf(intro);
				String answer = br.readLine();

				switch (answer) {
					case "1":
						System.out.printf(intro + "ID :  ");
						ID = br.readLine();
						screen_2();
						break;
					case "2":
						quit = true;
						break;
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void screen_2() throws IOException {
		try {
			boolean quit = false;
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			cons = new Consumer(ID);
			while (!quit) {
				System.out.printf("===============================\n");
				System.out.printf("Chatting Window\n");
				System.out.printf("1. List\n");
				System.out.printf("2. Make\n");
				System.out.printf("3. Join\n");
				System.out.printf("4. Log out\n");
				System.out.printf("===============================\n");

				System.out.printf(intro);
				String answer = br.readLine();

				switch (answer) {
					case "1":
						cons.getList();
						break;
					case "2":
						System.out.printf(intro + "Chat room name : ");
						String topic = br.readLine();

						// CREATE NEW TOPIC
						Properties props = new Properties();
						props.put(AdminClientConfig.BOOTSTRAP_SERVERS_CONFIG, "localhost:9092");

						try (Admin admin = Admin.create(props)) {
							String topicName = topic;
							int partitions = 1;
							short replicationFactor = 1;
							// Create a compacted topic
							CreateTopicsResult result = admin.createTopics(Collections.singleton(
									new NewTopic(topicName, partitions, replicationFactor)
											.configs(Collections.singletonMap(TopicConfig.CLEANUP_POLICY_CONFIG,
													TopicConfig.CLEANUP_POLICY_COMPACT))));

							// Call values() to get the result for a specific topic
							KafkaFuture<Void> future = result.values().get(topicName);

							// Call get() to block until the topic creation is complete or has failed
							// if creation failed the ExecutionException wraps the underlying cause.
							future.get();
						} catch (Exception e) {
						}

						System.out.printf(topic + " is created!\n");
						break;
					case "3":
						System.out.printf(intro + "Chat room name: ");
						String room = br.readLine();
						
						prod = new Producer(ID);
						cons.subscribe(room);
						screen_3(room);
						break;

					case "4":
						cons.destroy();
						ID = null;
						quit = true;
						break;
				}
			}
		} catch (IOException e) {
		}
	
	}

	public static void screen_3(String room) throws IOException{
		boolean quit = false;
		try {
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			while (!quit) {

				System.out.printf("===============================\n");
				System.out.printf("chat room : %s\n", room);
				System.out.printf("1. Read\n");
				System.out.printf("2. Write\n");
				System.out.printf("3. Reset\n");
				System.out.printf("4. Exit\n");
				System.out.printf("===============================\n");
				System.out.printf(intro);
				String answer = br.readLine();

				switch (answer) {
					case "1":
						cons.read();
						break;
					case "2":
						System.out.printf(intro + "Text: ");
						String msg = br.readLine();
						prod.send(room, ID, msg);
						break;

					case "3":
						cons.resetOffset();
						break;

					case "4":
						prod.destroy();
						quit = true;
						break;
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void main(String[] args) throws IOException{
		try {
			screen_1();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}

