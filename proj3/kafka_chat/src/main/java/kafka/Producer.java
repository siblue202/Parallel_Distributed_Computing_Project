package kafka;

import java.util.Collections;
import java.util.Properties;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.lang.*;
import java.util.*;

import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerConfig;
import org.apache.kafka.clients.producer.ProducerRecord;

public class Producer {
    KafkaProducer<String, String> prod;

    public void destroy() {
        this.prod.close();
    }

    public void send(String topic, String name, String msg) {
        ProducerRecord<String, String> record = new ProducerRecord<>(topic, name, msg);

        this.prod.send(record);
    }

    public Producer(String id) {
        
        Properties config = new Properties();
        config.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, "localhost:9092");
        config.put(ProducerConfig.CLIENT_ID_CONFIG, id);
        config.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG,
                "org.apache.kafka.common.serialization.StringSerializer");
        config.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG,
                "org.apache.kafka.common.serialization.StringSerializer");
        config.put(ProducerConfig.LINGER_MS_CONFIG, 1);
        prod = new KafkaProducer<>(config);        
    }
}

