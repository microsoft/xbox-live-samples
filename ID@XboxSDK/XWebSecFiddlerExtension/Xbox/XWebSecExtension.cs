using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using Fiddler;


namespace XWebSecFiddlerExtension
{
    public class XWebSecExtension : IAutoTamper
    {
        /// <summary>
        /// UI data structures
        /// </summary>
        private WpfHost viewControl = null;
        private TabPage tabPage = null;

        /// <summary>
        /// Domain data structures
        /// </summary>
        private Types.XWebSecData domainData = null;
        private ObservableCollection<String> errorList = null;

        /// <summary>
        /// Indicates whether an update check has been performed
        /// </summary>
        //private static bool updateCheck = false;

        /// <summary>
        /// Constructor
        /// </summary>
        public XWebSecExtension()
        {
        }

        /// <summary>
        /// Initializes UI and primary data structures.
        /// </summary>
        private void InitializeXWebSecUI()
        {
            try
            {
                domainData = new Types.XWebSecData();
                errorList = new ObservableCollection<string>();

                tabPage = new TabPage("Xbox Web Security");
                FiddlerApplication.UI.tabsViews.ImageList.Images.Add("xsec", GetTabIcon());
                tabPage.ImageIndex = FiddlerApplication.UI.tabsViews.ImageList.Images.IndexOfKey("xsec");
                viewControl = new WpfHost();
                viewControl.Dock = DockStyle.Fill;
                tabPage.Controls.Add(viewControl);

                String path = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "excludedDomains.xml");
                ParseExludedDomainsFile(path);

                FiddlerApplication.UI.tabsViews.TabPages.Add(tabPage);
                FiddlerApplication.AfterSessionComplete += XWebSecExtension_SessionComplete;
                FiddlerApplication.OnLoadSAZ += XWebSecExtension_Load;
            }
            catch (Exception exc)
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine("XWebSec Fiddler plugin encountered a fatal exception while loading:");
                sb.AppendLine(exc.ToString());
                MessageBox.Show(sb.ToString());
            }
        }

        /// <summary>
        /// Loads icon for tab display.
        /// </summary>
        private System.Drawing.Bitmap GetTabIcon()
        {
            Stream iconStream = Assembly.GetExecutingAssembly().GetManifestResourceStream(typeof(XWebSecExtension), "xbox.png");
            return new System.Drawing.Bitmap(iconStream);
        }

        /// <summary>
        /// Loads excluded domain list from a file and populates datastructure.
        /// </summary>
        bool ParseExludedDomainsFile(string filename)
        {
            XmlReader reader = XmlReader.Create(new StreamReader(filename));
            String domainName = null;
            String protocol = null;

            try
            {
                while (reader.ReadToFollowing("domain"))
                {
                    reader.MoveToAttribute("protocol");
                    protocol = reader.Value;
                    reader.MoveToAttribute("name");
                    domainName = reader.Value;
                    

                    if (protocol == "http" || protocol == "https")
                    {
                        domainData.filteredDomains.Add(new Types.XWebSecFilterDomain(new Regex(domainName.Replace("*.", "[a-zA-Z.]*(.)")), protocol == "https"));
                        FiddlerApplication.Log.LogString("[XWebSec] Loaded " + "(" + domainName + ") secure:" + (protocol == "https").ToString());
                    }
                    else
                    {
                        throw new UriFormatException();
                    }
                }

                FiddlerApplication.Log.LogString("[XWebSec] Loaded " + domainData.filteredDomains.Count + " entries into domain filter.");
                return true;
            }
            catch (FileNotFoundException)
            {
                FiddlerApplication.Log.LogString("[XWebSec] No domain filter file found: " + filename);
                return false;
            }
            catch (Exception exc)
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine("XWebSec Fiddler plugin encountered a fatal exception while loading exception list:");
                sb.AppendLine(exc.ToString());
                MessageBox.Show(sb.ToString());

                FiddlerApplication.Log.LogString("[XWebSec] Error loading domain filter!");
                return false;
            }
        }

        /// <summary>
        /// Imports sessions loaded by Fiddler from new SAZ into extension.
        /// </summary>
        void XWebSecExtension_Load(object sender, FiddlerApplication.ReadSAZEventArgs e)
        {
            viewControl.EnableLoadingOverlay();
            Session[] allSessions = e.arrSessions;

            try
            {
                FiddlerApplication.Log.LogString("[XWebSec] Loading " + allSessions.GetLength(0) + " sessions.");

                //Adding sessions like Fiddler without clearing
                domainData.addSessions(allSessions);
                viewControl.UpdateUI(domainData);
            }
            catch (Exception exc)
            {
                viewControl.DisableLoadingOverlay();
                MessageBox.Show($"Xbox Web Security Plugin encountered a fatal exception while loading: {exc.ToString()}");
            }

            viewControl.DisableLoadingOverlay();
        }

        /// <summary>
        /// Handles select action on Fiddler session.
        /// </summary>
        void XWebSecExtension_Select(string domain)
        {
#if (DEBUG)
            FiddlerApplication.Log.LogString($"[XWebSec] Selected [{domain}]");
#endif

            foreach (ListViewItem item in FiddlerApplication.UI.lvSessions.Items)
            {
                if (((Session)item.Tag).hostname == domain && ((Session)item.Tag).isTunnel == false)
                {
                    item.Selected = true;
                }
                else
                {
                    item.Selected = false;
                }
            }

        }

        /// <summary>
        /// Handles completed session and adds to extension data structures.
        /// </summary>
        private void XWebSecExtension_SessionComplete(Session session)
        {
            // skip over tunnels
            if (session.isTunnel)
                return;

            try
            {
                domainData.addSession(session);
                viewControl.UpdateUI(domainData);
            }
            catch (Exception exc)
            {
                StringBuilder sb = new StringBuilder();
                MessageBox.Show($"Xbox Web Security Plugin encountered a fatal exception while processing session: {exc.ToString()}");
            }
        }

        /// <summary>
        /// Respond to QuickExec commands
        /// </summary>
        /// <returns>Whether the command was handled or not</returns>
        public bool OnExecAction(string command)
        {
            command = command.ToLower();

            if (!command.StartsWith("xwebsec "))
                return false;

            command = command.Remove(0, 11);

            return true;
        }

        public void AutoTamperRequestBefore(Session oSession)
        {
        }

        public void AutoTamperRequestAfter(Session oSession)
        {
        }

        public void AutoTamperResponseBefore(Session oSession)
        {
        }

        public void AutoTamperResponseAfter(Session oSession)
        {
        }

        public void OnBeforeReturningError(Session oSession)
        {
        }

        public void OnLoad()
        {
            InitializeXWebSecUI();
        }

        public void OnBeforeUnload()
        {
        }
    }
}
