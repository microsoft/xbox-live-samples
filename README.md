## Welcome!

This repo contains the samples that demonstrate the API usage patterns for Microsoft Xbox Live Service.
These code samples target the Universal Windows Platform (UWP) and Xbox One XDK.
To get access to the Xbox Live service, you can join the Xbox Live Creators Program at https://aka.ms/xblcp, or apply to the ID@Xbox program at http://www.xbox.com/en-us/Developers/id

## How to clone repo

This repo contains submodules.  There are two ways to make sure you get submodules.

When initially cloning, make sure you use the "--recursive" option. IE:

    git clone --recursive https://github.com/Microsoft/xbox-live-samples.git

If you already cloned the repo, you can initialize submodules with:

    git submodule update --init

Note that using GitHub's feature to "Download Zip" instead of cloning does not contain the submodules and will not properly build.  Please clone recursively instead.

## How to run samples

You need to change the target devices sandbox to XDKS.1, and sign in with any Xbox Live enabled account.

For details on how to change sandbox for developers in the Xbox Live Creators Program, see: https://developer.microsoft.com/en-us/games/xbox/docs/xboxlive/get-started/creators/xbox-live-sandboxes-creators

For details on how to change sandbox for developers in ID@Xbox Program, see:
https://developer.microsoft.com/en-us/games/xbox/docs/xboxlive/get-started/xbox-live-sandboxes


## Contribute Back!

Is there a feature missing that you'd like to see, or found a bug that you have a fix for? Or do you have an idea or just interest in helping out in building the samples? Let us know and we'd love to work with you. For a good starting point on where we are headed and feature ideas, take a look at our [requested features and bugs](https://github.com/Microsoft/xbox-live-samples/issues).  

Big or small we'd like to take your contributions back to help improve the Xbox Live samples for everyone.

## Having Trouble?

We'd love to get your review score, whether good or bad, but even more than that, we want to fix your problem. If you submit your issue as a Review, we won't be able to respond to your problem and ask any follow-up questions that may be necessary. The most efficient way to do that is to open a an issue in our [issue tracker](https://github.com/Microsoft/xbox-live-samples/issues).  

Any questions you might have can be answered on the [MSDN Forums](https://social.msdn.microsoft.com/Forums/en-US/home?forum=xboxlivedev).  You can also ask programming related questions to [Stack Overflow](http://stackoverflow.com/questions/tagged/xbox-live) using the "xbox-live" tag.  The Xbox Live team will be engaged with the community and be continually improving our APIs, tools, and documentation based on the feedback received there.  

For developers in the Xbox Live Creators Program, you can submit a new idea or vote on existing idea at our [Xbox Live Creators Program User Voice](https://aka.ms/xblcpuv)

### Xbox Live GitHub projects
*   [Xbox Live Service API for C++](https://github.com/Microsoft/xbox-live-api)
*   [Xbox Live Service API for C#](https://github.com/Microsoft/xbox-live-api-csharp)
*   [Xbox Live Samples](https://github.com/Microsoft/xbox-live-samples)
*   [Xbox Live Unity Plugin](https://github.com/Microsoft/xbox-live-unity-plugin)
*   [Xbox Live Resiliency Fiddler Plugin](https://github.com/Microsoft/xbox-live-resiliency-fiddler-plugin)
*   [Xbox Live Trace Analyzer](https://github.com/Microsoft/xbox-live-trace-analyzer)

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
