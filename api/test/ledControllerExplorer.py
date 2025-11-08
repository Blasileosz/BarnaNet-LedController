import explorer
import alarmService	
import ledControllerService

if __name__ == "__main__":
	app = explorer.APIExplorer(BNservices=[alarmService.AlarmService, ledControllerService.LedControllerService], ip="192.168.0.80", port=2005)
	app.run()
