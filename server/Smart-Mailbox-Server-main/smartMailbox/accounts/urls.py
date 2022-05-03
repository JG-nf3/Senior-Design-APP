from unicodedata import name
from django.urls import include,path
from django.views.generic import TemplateView
from . import views

urlpatterns = [
    path('login_user', views.login_user, name='login_user'),
    path('inteface_login_user', views.interface_login_user, name='interface_login_user'),
    path('register_user', views.register_user, name='register_user'),
    path('logout_user', views.logout_user, name ='logout_user'),
    path('user_settings', views.user_settings, name = 'user_settings'),
    ]