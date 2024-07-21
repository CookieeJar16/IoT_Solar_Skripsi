package com.example.solariot;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import android.Manifest;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;


public class MainActivity extends AppCompatActivity {
    DatabaseReference mydb;
    TextView V0;
    TextView V1;
    TextView V2;
    TextView V3;
    TextView Batt;
    ImageView statusView;
    TextView Status;
    ImageView statusBatt;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {

                NotificationChannel channel = new NotificationChannel("My Notification", "My Notification", NotificationManager.IMPORTANCE_DEFAULT);
                NotificationChannel channel2 = new NotificationChannel("Notification Battery", "My Notification", NotificationManager.IMPORTANCE_DEFAULT);
                NotificationManager manager = getSystemService(NotificationManager.class);
                manager.createNotificationChannel(channel);
                manager.createNotificationChannel(channel2);
            }


        statusView = findViewById(R.id.img_7);
        statusBatt = findViewById(R.id.img_6);
        Status = findViewById(R.id.status);
        V0 = findViewById(R.id.val_1);
        V1 = findViewById(R.id.val_2);
        V2 = findViewById(R.id.val_3);
        V3 = findViewById(R.id.val_5);
        Batt = findViewById(R.id.val_6);


        mydb = FirebaseDatabase.getInstance().getReference().child("Sensor");
        try {
            mydb.addValueEventListener(new ValueEventListener() {
                @Override
                public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                    float V0data = dataSnapshot.child("V0").getValue(Float.class);
                    float V1data = dataSnapshot.child("V1").getValue(Float.class);
                    float V2data = dataSnapshot.child("V2").getValue(Float.class);
                    float V3data = dataSnapshot.child("V3").getValue(Float.class);
                    int Battdata = dataSnapshot.child("bat").getValue(Integer.class);

                    String V0text = String.format("%.02f", V0data);
                    String V1text = String.format("%.02f", V1data);
                    String V2text = String.format("%.02f", V2data);
                    String V3text = String.format("%.02f", V3data);
                    String Battext = String.valueOf(Battdata);

                    V0.setText(V0text + " V");
                    V1.setText(V1text + " V");
                    V2.setText(V2text + " A");
                    V3.setText(V3text + " W");
                    Batt.setText(Battext + " %");

                    if (V1data == 0) {
                        statusView.setImageResource(R.drawable.baseline_error_24);
                        Status.setText(R.string.notif_text_off);

                        NotificationCompat.Builder builder = new NotificationCompat.Builder(MainActivity.this, "My Notification");
                        builder.setContentTitle("Solar IoT");
                        builder.setContentText("Panel surya tidak terhubung");
                        builder.setSmallIcon(R.drawable.baseline_error_24);
                        builder.setAutoCancel(true);

                        NotificationManagerCompat managerCompat = NotificationManagerCompat.from(MainActivity.this);
                        if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                            return;
                        }
                        managerCompat.notify(1, builder.build());
                    }
                    else{
                        statusView.setImageResource(R.drawable.baseline_check_circle_24);
                        Status.setText(R.string.notif_text_on);
                    }
                    if (Battdata < 30 ){
                        statusBatt.setImageResource(R.drawable.baseline_battery_2_bar_24);

                        NotificationCompat.Builder builder = new NotificationCompat.Builder(MainActivity.this, "Notification Battery");
                        builder.setContentTitle("Solar IoT");
                        builder.setContentText("Baterai Lemah " + Battext + "%");
                        builder.setSmallIcon(R.drawable.baseline_error_24);
                        builder.setAutoCancel(true);

                        NotificationManagerCompat managerCompat = NotificationManagerCompat.from(MainActivity.this);
                        if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                            return;
                        }
                        managerCompat.notify(2, builder.build());

                    } else if (Battdata >=30 && Battdata <=85) {
                        statusBatt.setImageResource(R.drawable.baseline_battery_4_bar_24);
                    } else if (Battdata > 85) {
                        statusBatt.setImageResource(R.drawable.baseline_battery_full_24);
                    }
                }

                @Override
                public void onCancelled(@NonNull DatabaseError error) {

                }
            });
        } catch (Exception e)
        {
        }
    }
}