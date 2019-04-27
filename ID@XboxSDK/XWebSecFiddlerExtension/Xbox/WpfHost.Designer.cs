namespace XWebSecFiddlerExtension
{
    partial class WpfHost
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.wpfElementHost = new System.Windows.Forms.Integration.ElementHost();
            this.xsecFiddlerViewControl1 = new XWebSecFiddlerExtension.XWebSecFiddlerViewControl();
            this.SuspendLayout();
            // 
            // wpfElementHost
            // 
            this.wpfElementHost.AutoSize = true;
            this.wpfElementHost.Dock = System.Windows.Forms.DockStyle.Fill;
            this.wpfElementHost.Location = new System.Drawing.Point(0, 0);
            this.wpfElementHost.Name = "wpfElementHost";
            this.wpfElementHost.Size = new System.Drawing.Size(1066, 744);
            this.wpfElementHost.TabIndex = 0;
            this.wpfElementHost.Text = "elementHost1";
            this.wpfElementHost.Child = this.xsecFiddlerViewControl1;
            // 
            // WpfHost
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.Controls.Add(this.wpfElementHost);
            this.Name = "WpfHost";
            this.Size = new System.Drawing.Size(1066, 744);
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        #endregion

        private System.Windows.Forms.Integration.ElementHost wpfElementHost;
        private XWebSecFiddlerViewControl xsecFiddlerViewControl1;
    }
}
