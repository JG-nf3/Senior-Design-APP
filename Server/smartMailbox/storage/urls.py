from ast import Add
from django.urls import path
from . import views, api_views

urlpatterns = [path('api/update', api_views.updateBox, name='updateBox'),
                path('api/boxes/',api_views.allMailboxes.as_view(), name = 'getBoxes'),
                path('api/updateBox', api_views.addUpdate, name='addUpdate'),
                path('editBox/<int:pk>/',views.mailbox_edit, name='box_edit'),
                path('newBox/',views.mailbox_create, name='box_new'),
                path('interface2/', views.interface_user),
                path('api/userBoxes/<int:pk>/',api_views.userMailboxes, name = 'userGetBoxes'),
                path('api/userBox/<int:pk>/',api_views.userMailbox, name = 'userGetBox'),
                path('api/userUpdateBox/<int:pk>/',api_views.userUpdateMailbox, name = 'updateUserBox'),
                path('api/userDeleteBox/<int:pk>/',api_views.userDeleteMailbox, name = 'deleteUserBox')]