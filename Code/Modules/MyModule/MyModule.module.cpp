#include "Suora/Core/Log.h"

			//** Header Includes */
			#include "C:\Users\Megaport\Documents\GitHub\SuoraEngine\Code\Modules\MyModule\MyNode.generated.h"
#include "C:\Users\Megaport\Documents\GitHub\SuoraEngine\Code\Modules\MyModule\MyNode.h"


			inline extern void MyModule_Init()
			{
				SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, " - MyModule");
			}