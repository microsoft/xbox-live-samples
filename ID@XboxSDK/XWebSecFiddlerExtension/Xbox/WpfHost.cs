using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections.ObjectModel;
using System.Windows.Threading;

namespace XWebSecFiddlerExtension
{
    public partial class WpfHost : UserControl
    {
        XWebSecFiddlerViewControl xamlViewControl = null;

        public WpfHost()
        {
            InitializeComponent();
            xamlViewControl = wpfElementHost.Child as XWebSecFiddlerViewControl;
        }

        internal void UpdateUI(Types.XWebSecData domainData)
        {
#if (DEBUG)
            Fiddler.FiddlerApplication.Log.LogString($"[XWebSec] Counts: {domainData.httpDomainList.Count} {domainData.httpsDomainList.Count} {domainData.filteredDomainList.Count}");
#endif
            xamlViewControl.UpdateUI(domainData);
        }
        internal void EnableLoadingOverlay()
        {
            xamlViewControl.EnableLoadingOverlay();
        }
        internal void DisableLoadingOverlay()
        {
            xamlViewControl.DisableLoadingOverlay();
        }
    }
}
