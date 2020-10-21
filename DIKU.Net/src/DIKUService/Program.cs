using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

/***
 * Refer to:
 * https://docs.microsoft.com/en-us/dotnet/api/system.net.sockets.tcplistener?view=netcore-3.1
 **/
namespace DIKUService {
	static class Program {
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		static void Main ( ) {
			ServiceBase [] ServicesToRun;
			ServicesToRun = new ServiceBase []
			{
				new Service()
			};
			ServiceBase.Run ( ServicesToRun );
		}
	}
}
