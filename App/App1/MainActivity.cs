using Android.App;
using Android.Widget;
using Android.OS;
using System;
using Android.Content;
using Android.Runtime;
using Android.Views;
using Android.Bluetooth;
using System.Linq;


namespace App1
{
    [Activity(Label = "App1", MainLauncher = true, Icon = "@drawable/Bike")]
    public class MainActivity : Activity
    {

        BluetoothConnection myConnection = new BluetoothConnection();
        BluetoothSocket _socket = null;
        //BluetoothSocket _socket = null;





        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);

            myConnection.IsConnected = false;

            //Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);
            Button button_connect = FindViewById<Button>(Resource.Id.button1);
            //button.Click += delegate { button.Text = string.Format("NICE DUDE ! {0}", count++); };
            button_connect.Click += delegate { Connection(); };

            SeekBar pwm_bar = FindViewById<SeekBar>(Resource.Id.seekBar1);
            TextView _txt = FindViewById<TextView>(Resource.Id.textView1);
            pwm_bar.Max = 100;

            pwm_bar.ProgressChanged += (object sender, SeekBar.ProgressChangedEventArgs e) => {
                if (e.FromUser)
                {
                    int pwm_motor = e.Progress;
                    _txt.Text = string.Format("PWM = {0} %", pwm_motor);
                    Send_PWM(pwm_motor);
                }
            };


        }

        void Send_PWM(int pwm_motor)
        {
            if (myConnection.IsConnected==true)
            {
                byte[] a = new byte[1];
                a[0] = Convert.ToByte(pwm_motor * 2.55);
                myConnection.thisSocket.OutputStream.Write(a, 0, 1);

                
            }
            

        }

        void Connection()
        {
            
           
            myConnection.getAdapter();
            myConnection.thisAdapter.Enable();
            while (myConnection.thisAdapter.IsEnabled == false)
            { }
            myConnection.thisAdapter.StartDiscovery();

            try
            {

                myConnection.getDevice();
                myConnection.thisDevice.SetPairingConfirmation(false);
                myConnection.thisDevice.SetPairingConfirmation(true);
                myConnection.thisDevice.CreateBond();


            }
            catch (Exception deviceEX)
            {
            }

            myConnection.thisAdapter.CancelDiscovery();

          
            _socket = myConnection.thisDevice.CreateRfcommSocketToServiceRecord(Java.Util.UUID.FromString("00001101-0000-1000-8000-00805f9b34fb"));
            myConnection.thisSocket = _socket;

            try
            {
                
                myConnection.thisSocket.Connect();
                Toast.MakeText(this, "Connected!", ToastLength.Long).Show();
                myConnection.IsConnected = true;
              

            }
            catch (Exception CloseEX)
            {

            }

        }


    }

    public class BluetoothConnection
    {

        public void getAdapter() { this.thisAdapter = BluetoothAdapter.DefaultAdapter; }
        public void getDevice() { this.thisDevice = (from bd in this.thisAdapter.BondedDevices where bd.Name == "HC-05" select bd).FirstOrDefault(); }

        public BluetoothAdapter thisAdapter { get; set; }
        public BluetoothDevice thisDevice { get; set; }

        public BluetoothSocket thisSocket { get; set; }

        public bool IsConnected { get; set; }

        



    }
}

