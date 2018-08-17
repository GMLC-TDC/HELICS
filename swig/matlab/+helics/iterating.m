function v = iterating()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783852);
  end
  v = vInitialized;
end
