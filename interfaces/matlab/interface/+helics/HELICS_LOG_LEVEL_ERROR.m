function v = HELICS_LOG_LEVEL_ERROR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 58);
  end
  v = vInitialized;
end
