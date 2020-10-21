using System.ServiceProcess;

namespace DIKUService {
	internal class Service : ServiceBase {
		Service ( ) {
			this.CanPauseAndContinue = false;
			this.CanHandlePowerEvent = true;
		}

		protected override System.Boolean OnPowerEvent ( PowerBroadcastStatus powerStatus ) {
			return base.OnPowerEvent ( powerStatus );
		}
		protected override void OnStart ( System.String [] args ) {
			base.OnStart ( args );
		}

		protected override void OnStop ( ) {
			base.OnStop ();
		}
	}
}
