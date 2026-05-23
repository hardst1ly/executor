using System;
using System.Drawing;
using System.IO.Pipes;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ExecutorGUI
{
    public partial class MainForm : Form
    {
        private NamedPipeClientStream pipeClient;
        private Label statusLabel;
        
        // Designer elements - declare them here
        private RichTextBox richTextBox1;
        private Button button1;
        private Button button2;

        private readonly string[] scriptNames = {
            "Infinite Yield",
            "UNC Test", 
            "SUNC Test",
            "Vulnerability Test"
        };

        private readonly string[] scriptSources = {
            "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/EdgeIY/infiniteyield/master/source\"))()",
            "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/unified-naming-convention/NamingStandard/refs/heads/main/UNCCheckEnv.lua\"))()",
            "getgenv().sUNCDebug = {\n    [\"printcheckpoints\"] = false,\n    [\"delaybetweentests\"] = 0,\n    [\"printtesttimetaken\"] = false,\n}\n\nloadstring(game:HttpGet(\"https://script.sunc.su/\"))()",
            "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/zryr/Vulnerability-Check/refs/heads/main/Script\"))()"
        };

        public MainForm()
        {
            InitializeComponent();
            SetupCustomElements();
            InitializePipe();
        }

        private void SetupCustomElements()
        {
            // Setup existing elements from designer
            if (this.richTextBox1 != null)
            {
                this.richTextBox1.Font = new Font("Consolas", 10);
                this.richTextBox1.Text = "print('Hello from GUI!')";
            }
            
            // Add event handlers to designer buttons
            if (this.button1 != null)
                this.button1.Click += new EventHandler(this.ExecuteButton_Click);
            
            if (this.button2 != null)
                this.button2.Click += new EventHandler(this.InjectButton_Click);
            
            // Add status label
            this.statusLabel = new Label();
            this.statusLabel.Location = new Point(12, 320);
            this.statusLabel.Size = new Size(648, 20);
            this.statusLabel.Text = "Ready - Waiting for connection...";
            this.statusLabel.ForeColor = Color.Blue;
            this.Controls.Add(this.statusLabel);

            // Hotkeys
            this.KeyPreview = true;
            this.KeyDown += new KeyEventHandler(this.MainForm_KeyDown);
        }

        private async void InitializePipe()
        {
            await Task.Run(async () =>
            {
                while (true)
                {
                    try
                    {
                        pipeClient = new NamedPipeClientStream(".", "ExecutorPipe", PipeDirection.Out);
                        await Task.Run(() => pipeClient.Connect(1000));
                        
                        this.Invoke(new Action(() => {
                            if (statusLabel != null)
                            {
                                statusLabel.Text = "Connected to executor!";
                                statusLabel.ForeColor = Color.Green;
                            }
                        }));
                        break;
                    }
                    catch
                    {
                        await Task.Delay(2000);
                    }
                }
            });
        }

        private async void SendScript(string script)
        {
            try
            {
                if (pipeClient != null && pipeClient.IsConnected)
                {
                    byte[] data = Encoding.UTF8.GetBytes(script);
                    await Task.Run(() => pipeClient.Write(data, 0, data.Length));
                    await Task.Run(() => pipeClient.Flush());
                    
                    if (statusLabel != null)
                    {
                        statusLabel.Text = "Script sent successfully!";
                        statusLabel.ForeColor = Color.Green;
                    }
                }
                else
                {
                    if (statusLabel != null)
                    {
                        statusLabel.Text = "Not connected to executor!";
                        statusLabel.ForeColor = Color.Red;
                    }
                    InitializePipe(); // Try to reconnect
                }
            }
            catch (Exception ex)
            {
                if (statusLabel != null)
                {
                    statusLabel.Text = "Error: " + ex.Message;
                    statusLabel.ForeColor = Color.Red;
                }
            }
        }

        private async void SendCommand(string command)
        {
            try
            {
                if (pipeClient != null && pipeClient.IsConnected)
                {
                    byte[] data = Encoding.UTF8.GetBytes("CMD:" + command);
                    await Task.Run(() => pipeClient.Write(data, 0, data.Length));
                    await Task.Run(() => pipeClient.Flush());
                    
                    if (statusLabel != null)
                    {
                        statusLabel.Text = "Command sent: " + command;
                        statusLabel.ForeColor = Color.Green;
                    }
                }
                else
                {
                    if (statusLabel != null)
                    {
                        statusLabel.Text = "Not connected to executor!";
                        statusLabel.ForeColor = Color.Red;
                    }
                }
            }
            catch (Exception ex)
            {
                if (statusLabel != null)
                {
                    statusLabel.Text = "Error: " + ex.Message;
                    statusLabel.ForeColor = Color.Red;
                }
            }
        }

        private void ExecuteButton_Click(object sender, EventArgs e)
        {
            if (richTextBox1 == null || string.IsNullOrWhiteSpace(richTextBox1.Text))
            {
                if (statusLabel != null)
                {
                    statusLabel.Text = "No script to execute!";
                    statusLabel.ForeColor = Color.Orange;
                }
                return;
            }

            SendScript(richTextBox1.Text);
        }

        private void InjectButton_Click(object sender, EventArgs e)
        {
            SendCommand("INJECT");
            if (statusLabel != null)
            {
                statusLabel.Text = "Injection command sent!";
                statusLabel.ForeColor = Color.Blue;
            }
        }

        private void MainForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Insert)
            {
                this.WindowState = this.WindowState == FormWindowState.Minimized 
                    ? FormWindowState.Normal 
                    : FormWindowState.Minimized;
            }
            else if (e.KeyCode == Keys.F1)
            {
                // Quick inject hotkey
                InjectButton_Click(sender, e);
            }
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            if (pipeClient != null)
                pipeClient.Close();
            base.OnFormClosed(e);
        }

        private void InitializeComponent()
        {
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(12, 12);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.Size = new System.Drawing.Size(648, 268);
            this.richTextBox1.TabIndex = 0;
            this.richTextBox1.Text = "";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 286);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 1;
            this.button1.Text = "execute";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(585, 286);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 2;
            this.button2.Text = "inject";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // MainForm
            // 
            this.ClientSize = new System.Drawing.Size(672, 312);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.richTextBox1);
            this.Name = "MainForm";
            this.ResumeLayout(false);

        }
    }
}