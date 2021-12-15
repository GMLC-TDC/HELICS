function v = HELICS_LOG_LEVEL_TRACE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 68);
  end
  v = vInitialized;
end
