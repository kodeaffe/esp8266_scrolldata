from datetime import datetime

from django.conf import settings
from django.http import JsonResponse
from django.utils.decorators import method_decorator
from django.views.decorators.csrf import csrf_exempt
from django.views.generic import TemplateView


class JSONResponseMixin(object):
    def render_to_response(self, context, **response_kwargs):
        return JsonResponse(self.get_data(context), **response_kwargs)

    def get_data(self, context):
        return context


@method_decorator(csrf_exempt, name='dispatch')
class ControlView(JSONResponseMixin, TemplateView):
    def post(self, *args, **kwargs):
        settings.STATE = self.kwargs['state']
        return self.get(*args, **kwargs)

    def get_data(self, context):
        return {'state': getattr(settings, 'STATE', 0)}


class DataView(JSONResponseMixin, TemplateView):
    def get_data(self, context):
        now = datetime.now().strftime('%Y-%m-%d %H%M%S')
        return {
            'datetime': now,
            'state': getattr(settings, 'STATE', 0),
        }


class IndexView(TemplateView):
    template_name = 'index.html'


class LightsView(JSONResponseMixin, TemplateView):
    def get_data(self, context):
        return getattr(settings, 'LIGHTS', {})


class LightMixin(JSONResponseMixin):
    light = None

    def set_value(self):
        """This should be implemented by child"""
        return

    def post(self, *args, **kwargs):
        lights = getattr(settings, 'LIGHTS', {})
        try:
            self.light = lights[self.kwargs['light_id']]
            self.set_value()
        except KeyError:
            pass
        return self.get(*args, **kwargs)

    def get_data(self, context):
        return self.light


@method_decorator(csrf_exempt, name='dispatch')
class LightSetBrightnessView(LightMixin, TemplateView):
    def set_value(self):
        self.light['brightness'] = self.kwargs['brightness']


@method_decorator(csrf_exempt, name='dispatch')
class LightToggleView(LightMixin, TemplateView):
    def set_value(self):
        self.light['state'] = not self.light['state']
