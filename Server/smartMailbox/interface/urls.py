from django.urls import path
from django.views.generic import TemplateView
from .views import mailboxList

urlpatterns = [
    path('', mailboxList, name='interface'),  
    path('change_passcode/', TemplateView.as_view(template_name='home/change_passcode.html'), name='change_passcode'),
    path('box_activity/', TemplateView.as_view(template_name='home/box_activity.html'), name='box_activity'),
]
