function v = HELICS_USER_EXCEPTION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 72);
  end
  v = vInitialized;
end
