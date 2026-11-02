#include "ui/StatusItem.h"

#import <AppKit/AppKit.h>

void activateMacStatusItem(void *item);

@interface TTStatusItemTarget : NSObject
@property (nonatomic, assign) void *owner;
- (void)clicked:(id)sender;
@end

@implementation TTStatusItemTarget
- (void)clicked:(id)sender
{
    activateMacStatusItem(self.owner);
}
@end

namespace {

class MacStatusItem : public StatusItem
{
public:
    MacStatusItem()
    {
        m_item = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        m_target = [[TTStatusItemTarget alloc] init];
        m_target.owner = this;
        m_item.button.target = m_target;
        m_item.button.action = @selector(clicked:);
    }

    ~MacStatusItem() override
    {
        [[NSStatusBar systemStatusBar] removeStatusItem:m_item];
    }

    void setTitle(const QString &title) override
    {
        m_item.button.title = [NSString stringWithUTF8String:title.toUtf8().constData()];
    }

    void activate()
    {
        emit clicked();
    }

private:
    NSStatusItem *m_item = nil;
    TTStatusItemTarget *m_target = nil;
};

}

void activateMacStatusItem(void *item)
{
    static_cast<MacStatusItem *>(item)->activate();
}

std::unique_ptr<StatusItem> createStatusItem()
{
    return std::make_unique<MacStatusItem>();
}
