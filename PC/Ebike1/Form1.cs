using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace Ebike1
{
    public partial class Form1 : Form
    {
        int Message_Frame = 26;  //number of bytes in a message
        int[] Message_Tempo_Buffer = new int[100];
        int Message_Tempo_Buffer_size = 0;
        int[] Core_Data_buffer = new int[16];
        int current_cmd;
        int mode_cmd = 3; 
        int pwm_cmd;
        int action_cmd = 0; //no action

        double Wheel_Radius = 0.3; //[m]
        double Timestamp = 0;
        int pwm_back = 0;
        string mode_back = "";
        double position = 0;
        double speed = 0;
        double acc = 0;
        double voltage = 0;
        double current = 0;
        double Power = 0;
        string Torque_txt = "";
        bool Data_Received = false;

        int Logger_tic=0;
        int Logger_Sampling=1;
        StreamWriter File = new StreamWriter("DATA_LOG.csv");
        
        
        public Form1()
        {
            InitializeComponent();
            File.Write("TimeStamp[ms];Data Received;Mode;PWM[%];Position[m];Speed[m/s];Acceleration[m/s2];Voltage[V];Current[A];Power[W];Torque[nm] \r\n");
            radioButton1.Checked = true;
            timer1.Start();
            radioButton3.Select();

        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            serialPort1.PortName = "COM3";
            serialPort1.BaudRate = 38400;
            serialPort1.Open();
            label1.Text = "State : Connected";
        }

        private void button2_Click(object sender, EventArgs e)
        {

            serialPort1.Close();
            label1.Text = "State : Disconnected";

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (label1.Text == "State : Connected")
            {

                Com_read();
                Control_Action();
                Com_write();
                

                Timestamp = Timestamp + timer1.Interval;

                if (checkBox1.Checked == true) // logger enabled
                {
                    if (Logger_tic >= Logger_Sampling)
                    {

                        File.Write(Timestamp.ToString() + ";" + Data_Received.ToString() + ";" + mode_back.ToString() + ";" + pwm_back.ToString() + ";" + position.ToString() + ";" + speed.ToString() + ";" + acc.ToString() + ";" + voltage.ToString() + ";" + current.ToString() + ";" + Power.ToString() + ";" + Torque_txt + "\r\n");
                        Logger_tic = 0;

                    }


                }

                Logger_tic++;


            }
        }

        private void Control_Action()
        {
            if (mode_cmd == 2) //Current control
            {
                double current_tg = Convert.ToDouble(trackBar1.Value) / 100;
                label8.Text = "Current_cmd : " + current_tg.ToString() + " A";
                current_cmd = Convert.ToInt16(100 * current_tg);
            }

            if (mode_cmd == 1)  //PWM
            {
                double pwm_tg = Math.Round(Convert.ToDouble(trackBar1.Value) / 2.55);
                label8.Text = "PWM_cmd : " + pwm_tg.ToString() + " %";
                pwm_cmd = Convert.ToInt16(2.55 * pwm_tg);
            }

            if (mode_cmd == 3)  //bluetooth
            {
                pwm_cmd = 0;
            }

        }

        private void Com_write()
        {
            byte[] sender = new byte[6] { 95, Convert.ToByte(mode_cmd),Convert.ToByte(current_cmd), Convert.ToByte(pwm_cmd), Convert.ToByte(action_cmd),222 };
            richTextBox1.Text = sender[4].ToString();
            serialPort1.Write(sender,0,6);
            action_cmd = 0; //action has been sent

        }
        private void Com_read()
        {
            Data_Received = false;
            label2.Text = "status :";
            
            // MessageBox.Show(serialPort1.BytesToRead.ToString());
            int BytesToRead = serialPort1.BytesToRead;

            int trash = 0;
            if (BytesToRead > 4*Message_Frame)
            {
                for (int u1=0; u1 < BytesToRead - 2*Message_Frame; u1++)
                { trash = serialPort1.ReadByte(); }
            }
            BytesToRead = serialPort1.BytesToRead;


            if (BytesToRead>=2*Message_Frame)
            {
                for (int i=0; i< Message_Frame-1; i++)
                {
                    Message_Tempo_Buffer[i] = serialPort1.ReadByte();
                }

                for (int y = 0; y < Message_Frame; y++)
                {
                    Message_Tempo_Buffer[y+ Message_Frame-1] = serialPort1.ReadByte();

                    if (Message_Tempo_Buffer[y] == 95 && Message_Tempo_Buffer[y + Message_Frame - 1] == 222)
                    {
                        label2.Text = "status : OK";
                        Data_Received = true;
                        Analyse_Message(Message_Tempo_Buffer,y);
                        //richTextBox1.Text =y.ToString() +" "+ Message_Tempo_Buffer[y].ToString() + " ; " + Message_Tempo_Buffer[y+1].ToString() + " ; " + Message_Tempo_Buffer[y+15].ToString();
                        break;
                    }
                }
                
            }
            else
            { label2.Text = "status : ERROR"; }
            

        }

        private void Analyse_Message(int[] buffer, int y)
        {

            pwm_back = (Convert.ToInt16(Convert.ToDouble(buffer[y + 1]) / 255 * 100));
            label3.Text = "PWM : " + Convert.ToString(pwm_back)+" %";

            label4.Text = "Position : " + buffer[y+2].ToString();

            byte[] B = new byte[4] { Convert.ToByte(buffer[y+6]), Convert.ToByte(buffer[y + 5]), Convert.ToByte(buffer[y + 4]), Convert.ToByte(buffer[y + 3]) };
            position = BitConverter.ToInt32(B,0);
            position = Math.Round((position / 1000),3);
            label5.Text = "Distance : "+position.ToString()+ " m";

            B = new byte[4] { Convert.ToByte(buffer[y + 10]), Convert.ToByte(buffer[y + 9]), Convert.ToByte(buffer[y + 8]), Convert.ToByte(buffer[y + 7]) };
            speed = BitConverter.ToInt32(B, 0);
            speed = Math.Round((speed / 1000), 3);
            label6.Text = "Speed : " + speed.ToString()+" m/s";

            if (speed>=0 && speed <= 16)
            { progressBar1.Value = Convert.ToInt16(Math.Round(speed * 3.6)); }
            //else
            //{ MessageBox.Show(buffer[y].ToString()+","+ buffer[y+7].ToString() + "," + buffer[y+25].ToString()); }
            label20.Text = Math.Round(speed * 3.6, 2).ToString() + " km/h";


            B = new byte[4] { Convert.ToByte(buffer[y + 14]), Convert.ToByte(buffer[y + 13]), Convert.ToByte(buffer[y + 12]), Convert.ToByte(buffer[y + 11]) };
            acc = BitConverter.ToInt32(B, 0);
            acc = Math.Round((acc / 1000), 3);
            label7.Text = "Acceleration : " + acc.ToString() +" m/s²";

            B = new byte[4] { Convert.ToByte(buffer[y + 18]), Convert.ToByte(buffer[y + 17]), Convert.ToByte(buffer[y + 16]), Convert.ToByte(buffer[y + 15]) };
            voltage = BitConverter.ToInt32(B, 0);
            voltage = Math.Round((voltage / 1000), 3);
            label10.Text = "Battery Voltage : " + voltage.ToString() +"V";

            B = new byte[4] { Convert.ToByte(buffer[y + 22]), Convert.ToByte(buffer[y + 21]), Convert.ToByte(buffer[y + 20]), Convert.ToByte(buffer[y + 19]) };
            current = BitConverter.ToInt32(B, 0);
            current = Math.Round((current / 1000), 3);
            label11.Text = "Motorization Current : " + current.ToString() + " A";

            label14.Text = "Current_cmd : " + (Convert.ToDouble(buffer[y + 23])/100).ToString()+ " A";
            
            if (buffer[y + 24]==1)
            { label9.Text = "Mode : PWM direct control"; mode_back = "PWM direct control"; }
            if (buffer[y + 24] == 2)
            { label9.Text = "Mode : Current control"; mode_back = "Current control"; }

            Power = Math.Round((current * voltage), 3);
            label12.Text = "Power : " + Power.ToString()+" W";
            if (speed > 0.1)
            {
                Torque_txt = Math.Round((current * voltage / speed) * Wheel_Radius, 3).ToString();
            }

            else
            {   Torque_txt= "---";
                
            }

            label13.Text = "Torque : " + Torque_txt + " Nm";



        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            
           radioManagement();
            
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {

            radioManagement();
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {

            radioManagement();
        }

        private void radioManagement()
        {
            if (radioButton1.Checked == true)
            {
                mode_cmd = 1;
                label8.Text = "PWM_cmd";
                trackBar1.Enabled = true;
            }

            if (radioButton2.Checked == true)
            {
                mode_cmd = 2;
                label8.Text = "Current_cmd";
                trackBar1.Enabled = true;
            }

            if (radioButton3.Checked == true)
            {
                mode_cmd = 3;
                label8.Text = "Bluetooth";
                trackBar1.Enabled = false;
            }

        }

        private void button3_Click(object sender, EventArgs e)
        {
            action_cmd = 147; //reset
        }

        private void button4_Click(object sender, EventArgs e)
        {
            action_cmd = 12; //LED ON
        }

        private void button5_Click(object sender, EventArgs e)
        {
            action_cmd = 33; //LED OFF
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked == true)
            {
                textBox1.Enabled = false;
                Logger_Sampling = Convert.ToInt16(textBox1.Text);
                Logger_tic = 0;
                button6.Enabled = false;


            }

            else
            {
                textBox1.Enabled = true;
                button6.Enabled = true;


            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            File.Close();
            textBox1.Enabled = false;
            checkBox1.Enabled = false;
            checkBox1.Checked = false;
            button6.Enabled = false;
        }

        
    }
}
