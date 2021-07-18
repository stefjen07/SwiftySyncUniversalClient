#include <GoogleAuthorization.h>
#include <FacebookAuthorization.h>
#include <Authorization.h>
#include <SwiftySyncUniversalClient.h>

using namespace std;

int main() {
	SwiftyUniversalClient client("ws://localhost:8888");
	
	auto facebookProvider = FacebookProvider();
	auto castedFacebookProvider = dynamic_cast<AuthorizationProvider*>(&facebookProvider);
	
	auto googleProvider = GoogleProvider();
	auto castedGoogleProvider = dynamic_cast<AuthorizationProvider*>(&googleProvider);
	
	client.supportedProviders = {
		castedFacebookProvider,
		castedGoogleProvider
	};

	client.authHandler = [](AuthorizationStatus status) {
		if (status == AuthorizationStatus::authorized) {
			cout << "Authorized successfully\n";
		}
	};

	client.run();

	client.authorize(0, "");

	while (true);
	return 0;
}