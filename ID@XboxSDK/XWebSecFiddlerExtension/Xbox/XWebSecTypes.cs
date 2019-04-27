using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using Fiddler;
using System.Windows.Forms;
using System.ComponentModel;

namespace XWebSecFiddlerExtension.Types
{
    public class XWebSecDomain
    {
        public String domainName { get; set; }

        public bool isHttps { get; set; }

        public bool isWebSocket { get; set; }

        public int count { get; set; }

        public String webSocketFormatted
        {
            get
            {
                if (isWebSocket)
                    return "yes";
                else
                    return "";
            }
        }
        public String httpsFormatted
        {
            get
            {
                if (isHttps)
                    return "HTTPS";
                else
                    return "HTTP";
            }
        }

        public XWebSecDomain(String domainname, bool wsocket, bool ishttps)
        {
            domainName = domainname;
            isWebSocket = wsocket;
            isHttps = ishttps;
            count = 0;
        }

        public int Compare(XWebSecDomain d1, XWebSecDomain d2)
        {
            if (d1.domainName == d2.domainName)
                return 1;

            return 0;
        }
    }

    public class XWebSecComparer : IEqualityComparer<XWebSecDomain>
    {
        public bool Equals(XWebSecDomain d1, XWebSecDomain d2)
        {
            return (d1.domainName == d2.domainName && d1.isHttps == d2.isHttps && d1.isWebSocket == d2.isWebSocket);
        }

        public int GetHashCode(XWebSecDomain domain)
        {
            return domain.domainName.GetHashCode() + domain.isHttps.GetHashCode() + domain.isHttps.GetHashCode();
        }
    }

    public class XWebSecFilterDomain
    {
        public Regex regex { get; set; }

        public bool isHttps { get; set; }

        public XWebSecFilterDomain(Regex domainFilter, bool ishttps)
        {
            regex = domainFilter;
            isHttps = ishttps;
        }
    }

    public class XWebSecData
    {
        public int httpDomainCount = 0;
        public int httpsDomainCount = 0;
        public int filteredDomainCount = 0;

        public Dictionary<XWebSecDomain, int> httpDomainList = new Dictionary<XWebSecDomain, int>(new XWebSecComparer());
        public Dictionary<XWebSecDomain, int> httpsDomainList = new Dictionary<XWebSecDomain, int>(new XWebSecComparer());
        public Dictionary<XWebSecDomain, int> filteredDomainList = new Dictionary<XWebSecDomain, int>(new XWebSecComparer());

        public List<Types.XWebSecFilterDomain> filteredDomains = new List<XWebSecFilterDomain>();

        public XWebSecData()
        {
        }

        /// <summary>
        /// Clears all internal data structures.
        /// </summary>
        public void clear()
        {
            httpDomainList.Clear();
            httpsDomainList.Clear();
            filteredDomainList.Clear();
            filteredDomains.Clear();

            httpDomainCount = 0;
            httpsDomainCount = 0;
            filteredDomainCount = 0;
        }

        /// <summary>
        /// Adds Fiddler session array into data structures
        /// </summary>
        /// <param name="sessions"></param>
        public void addSessions(Session[] sessions)
        {
            // adding sessions without clearing
            int i = 0;

#if (DEBUG)
            FiddlerApplication.Log.LogString($"[XWebSec] Session Update Count {sessions.Length}");
#endif

            foreach (Session s in sessions)
            {
                i++;
                addSession(s);
            }
        }

        /// <summary>
        /// Adds single Fiddler session into data structures.
        /// </summary>
        /// <param name="session"></param>
        public void addSession(Session session)
        {
            if (!session.isTunnel)
            {
                XWebSecDomain domain = new XWebSecDomain(session.hostname, session.bHasWebSocketMessages, session.isHTTPS);

                // pull out filtered domains first
                if (isInFilterList(session.hostname, session.isHTTPS))
                {
                    if (filteredDomainList.ContainsKey(domain))
                    {
                        filteredDomainList[domain] += 1;
#if (DEBUG)
                        FiddlerApplication.Log.LogString($"[XWebSec] Filtered Domains: (+1) [{domain.domainName}]");
#endif
                    }
                    else
                    {
                        filteredDomainList.Add(domain, 1);
                        filteredDomainCount++;
#if (DEBUG)
                        FiddlerApplication.Log.LogString($"[XWebSec] Filtered Domains: (add) [{ domain.domainName}]");
#endif
                    }
                }
                else
                {
                    if (session.isHTTPS)
                    {
                        if (httpsDomainList.ContainsKey(domain))
                        {
                            httpsDomainList[domain] += 1;
#if (DEBUG)
                            FiddlerApplication.Log.LogString($"[XWebSec] Https Domains: (+1) [{domain.domainName}]");
#endif
                        }
                        else
                        {
                            httpsDomainList.Add(domain, 1);
                            httpsDomainCount++;
#if (DEBUG)
                            FiddlerApplication.Log.LogString($"[XWebSec] Https Domains: (add) [{domain.domainName}]");
#endif
                        }
                    }
                    else
                    {
                        if (httpDomainList.ContainsKey(domain))
                        {
                            httpDomainList[domain] += 1;
#if (DEBUG)
                            FiddlerApplication.Log.LogString($"[XWebSec] Http Domains: (+1) [{domain.domainName}]");
#endif
                        }
                        else
                        {
                            httpDomainList.Add(domain, 1);
                            httpDomainCount++;
#if (DEBUG)
                            FiddlerApplication.Log.LogString($"[XWebSec] Http Domains: (add) [{domain.domainName}]");
#endif
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Returns if domain name (and protocol) is in data structures.
        /// </summary>
        /// <param name="domainName"></param>
        /// <param name="isHttps"></param>
        /// <returns></returns>
        public bool isInFilterList(string domainName, bool isHttps)
        {
            foreach (Types.XWebSecFilterDomain d in filteredDomains)
            {
                if (d.regex.IsMatch(domainName) && d.isHttps == isHttps)
                {
#if (DEBUG)
                    FiddlerApplication.Log.LogString($"[XWebSec] Filter match: {domainName}");
#endif
                    return true;
                }
            }

            return false;
        }
    }
}
