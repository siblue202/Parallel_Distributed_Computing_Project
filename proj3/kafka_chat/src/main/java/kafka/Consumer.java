package kafka;

import java.util.Collections;
import java.util.Properties;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.time.Duration;
import java.lang.*;
import java.util.*;

import org.apache.kafka.clients.consumer.ConsumerConfig;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.KafkaConsumer;
import org.apache.kafka.clients.admin.ListTopicsResult;
import org.apache.kafka.common.PartitionInfo;

public class Consumer {
    KafkaConsumer<String, String> cons;
   
    public void read() {
        ConsumerRecords<String, String> records = this.cons.poll(Duration.ofMillis(1000));
        this.cons.commitSync();
        for (ConsumerRecord<String, String> record : records)
		{
            System.out.printf("%s: %s\n", record.key(), record.value());
	    }
    }

    public void destroy() {
        this.cons.close();
    }

    public void subscribe(String topic) {
        this.cons.subscribe(Collections.singletonList(topic));
    }

    public void unsubscribe() {
        this.cons.unsubscribe();
    }

    public void resetOffset() {
        this.cons.seekToBeginning(this.cons.assignment());
    }

    public void getList() {
        Map<String, List<PartitionInfo> > topics = this.cons.listTopics();
        Set<String> keySet = topics.keySet();
        for (String key : keySet) {
            if (key.equals("__consumer_offsets")) continue;
            System.out.println(key);
        }
    }

    public Consumer(String id){
		Properties config = new Properties();
		config.put(ConsumerConfig.GROUP_ID_CONFIG, id);
		config.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, "localhost:9092");
		config.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.StringDeserializer");
		config.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.StringDeserializer");
		config.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG,"earliest");
		config.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, "false");

		this.cons = new KafkaConsumer<>(config);
	}
}

