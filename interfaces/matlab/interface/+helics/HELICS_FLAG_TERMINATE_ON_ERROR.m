function v = HELICS_FLAG_TERMINATE_ON_ERROR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 50);
  end
  v = vInitialized;
end
